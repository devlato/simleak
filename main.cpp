/**
 * (c) 2019 devlato <github@devlato.com>
 *
 * https://github.com/devlato
 */

#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <signal.h>

// (c) @jarro2783
// https://github.com/jarro2783/cxxopts
#include "cxxopts.hpp"

// Some typedefs
typedef uint8_t byte_t;
typedef byte_t* byteptr_t;
typedef size_t memsize_t;
typedef size_t index_t;
typedef std::tuple<index_t, memsize_t> scheduled_allocation_t;
typedef std::ostream* ostreamptr_t;

#define BYTE_MAX UINT8_MAX

const memsize_t KB_IN_BYTES = 1024;
const memsize_t MB_IN_BYTES = 1024 * KB_IN_BYTES;

// Default amount of memory to request
const memsize_t DEFAULT_MEMORY_LIMIT_IN_MB = 32 * 1024;

// We request memory in chunks to increase chances, so it's a chunk size
const memsize_t DEFAULT_CHUNK_SIZE_IN_MB = 32;

class null_buf : public std::stringbuf
{
  public:
    int sync() {
      // Do nothing
      return 0;
    }
};

// A vector for storing the pointers to allocated memory
std::vector<byteptr_t> allocated_memory_pointers;
// ostream for mandatory logging
ostreamptr_t console = &std::cout;
// ostream for debug/optional logging
ostreamptr_t debug = &std::cout;
// Flag showing if we need to fill out the buffer with zeroes
// instead of filling out random numbers
bool is_fill_with_zeroes_enabled = false;

// TODO Use SIMD to vectorize the randomization
void fill_with_random(byteptr_t buffer, memsize_t buffer_size) {
  *debug << "Filling out the memory block with random data...";
  std::srand(unsigned(std::time(nullptr)));

  for (index_t i = 0; i < buffer_size; i++) {
    buffer[i] = std::rand() % BYTE_MAX;
  }
  *debug << "success." << std::endl;
}

void fill_with_zeroes(byteptr_t buffer, memsize_t buffer_size) {
  *debug << "Filling out the memory block with zeroes...";
  std::memset(buffer, 0, buffer_size);
  *debug << "success." << std::endl;
}

float to_megabytes(memsize_t size_in_bytes) {
  return (float) size_in_bytes / (float) MB_IN_BYTES;
}

std::string as_megabytes(memsize_t size_in_bytes) {
  std::ostringstream stream;
  stream << std::defaultfloat << to_megabytes(size_in_bytes) << " MB";
  return stream.str();
}

void consume_memory(memsize_t memory_limit, memsize_t chunk_size) {
  index_t chunks_amount = memory_limit / chunk_size + (memory_limit % chunk_size == 0 ? 0 : 1);
  memsize_t last_chunk_size = memory_limit - (chunks_amount - 1) * chunk_size;

  *console << "Starting to consume memory..." << std::endl;
  *debug << "- Expected number of chunks: " << chunks_amount << std::endl
         << "- Expected last chunk size: " << as_megabytes(last_chunk_size) << std::endl << std::endl;

  allocated_memory_pointers.reserve(chunks_amount);
  std::queue<scheduled_allocation_t> scheduled_allocation_chunks;

  *console << "Queuing memory allocations..." << std::endl;
  for (index_t i = 0; i < chunks_amount; i++) {
    memsize_t current_chunk_size = i < chunks_amount - 1 ? chunk_size : last_chunk_size;
    *debug << "Queuing allocation of chunk #" << i << " (size: " << as_megabytes(current_chunk_size) << ")...";
    scheduled_allocation_chunks.push(scheduled_allocation_t(i, current_chunk_size));
    *debug << "success." << std::endl;
  }
  *console << std::endl;

  *console << "Executing scheduled allocations..." << std::endl;
  while (!scheduled_allocation_chunks.empty()) {
    auto& [chunk_index, current_chunk_size] = scheduled_allocation_chunks.front();
    scheduled_allocation_chunks.pop();
    *debug << "Allocating chunk #" << chunk_index << " (size: " << as_megabytes(current_chunk_size) << ")...";
    byteptr_t allocated_chunk = new (std::nothrow) byte_t[current_chunk_size];
    if (!allocated_chunk) {
      *debug << "failed. Rescheduling allocation..." << std::endl;
      scheduled_allocation_chunks.push(scheduled_allocation_t(chunk_index, current_chunk_size));
    } else {
      *debug << "success." << std::endl;
      allocated_memory_pointers[chunk_index] = allocated_chunk;
      if (is_fill_with_zeroes_enabled) {
        fill_with_zeroes(allocated_chunk, current_chunk_size);
      } else {
        fill_with_random(allocated_chunk, current_chunk_size);
      }
    }
  }

  *console << std::endl;
  *console << "All the required memory has been consumed." << std::endl;
}

void free_memory() {
  *console << "Releasing the allocated memory...";
  for (index_t i = 0; i < allocated_memory_pointers.size(); i++) {
    auto pointer = allocated_memory_pointers[i];
    *debug << std::endl << "Deleting memory for chunk #" << i << "...";
    delete[] pointer;
  }
  *console << "success." << std::endl;
}

void handle_sigterm(int signal) {
  *console << std::endl << "Ctrl+C has been pressed, exiting..." << std::endl;
  free_memory();

  exit(0);
}

void set_signal_handler() {
  signal(SIGINT, handle_sigterm);
}

int main(int argc, char * argv[]) {
  set_signal_handler();

  cxxopts::Options options("simleak", "Simulates a memory leak by consuming memory in small chunks");
  options
    .add_options()
    (
      "m,memory-limit",
      "Sets memory limit in MBs",
      cxxopts::value<memsize_t>()->default_value(std::to_string(DEFAULT_MEMORY_LIMIT_IN_MB))
    )
    (
      "c,chunk-size",
      "Sets chunk size in MBs",
      cxxopts::value<memsize_t>()->default_value(std::to_string(DEFAULT_CHUNK_SIZE_IN_MB))
    )
    (
      "v,verbose",
      "Enable verbose execution log"
    )
    (
      "z,fill-with-zeroes",
      "Fill the allocated memory with zeroes instead of random values (faster)"
    )
    (
      "h,help",
      "Print this help message"
    );

  auto args = options.parse(argc, argv);
  if (args["h"].as<bool>()) {
    *console << options.help();
    return 0;
  }

  auto memory_limit = args["m"].as<memsize_t>() * MB_IN_BYTES;
  auto chunk_size = args["c"].as<memsize_t>() * MB_IN_BYTES;
  auto is_verbose = args["v"].as<bool>();
  if (!is_verbose) {
    debug = new std::ostream(new null_buf());
  }
  is_fill_with_zeroes_enabled = args["z"].as<bool>();

  *console << "Memory options" << std::endl
           << "- Memory limit: " << as_megabytes(memory_limit) << std::endl
           << "- Chunk size: " << as_megabytes(chunk_size) << std::endl
           << "- Fill mode: " << (is_fill_with_zeroes_enabled ? "Zeroes" : "Random Values") << std::endl << std::endl;

  *console << "[?] Want to see help? Run this command with -h flag" << std::endl << std::endl;

  consume_memory(memory_limit, chunk_size);

  *console << "Memory allocated. Now we'll be just waiting here. When you need to exit, just press Ctrl+C." << std::endl;
  while (true);

  return 0;
}
