# Ripper IO Core

**Platform-independent I/O abstractions for the Ripper ecosystem.**

io-core provides a uniform interface for reading and writing data across
different backends — files, memory buffers, and beyond. It is the foundational
I/O layer for all Ripper libraries, designed with modularity and
single-responsibility in mind.

## Use cases

- Swap between file and memory I/O without changing application logic
- Build tools that read structured data from files and write results to memory
  (or vice versa)
- Write test suites that operate entirely in memory, avoiding filesystem noise
- Serve as a portable I/O backend for higher-level format libraries (PDF,
  images, archives, etc.)

## Documentation

- [User guide](docs/USAGE.md) — building, installing, and CMake integration
- [Contributor guide](docs/CONTRIBUTING.md) — building from source, running
  tests, formatting, and static analysis

## License

This library is distributed under the **MIT License**. You are free to use,
modify, and distribute it in any project, proprietary or open-source. See
[LICENSE](LICENSE) for the full text.

## Contact

If this library fits your use case, or if you have ideas for features or
improvements, we would love to hear from you. Open an issue or start a
discussion on the [GitHub repository](https://github.com/ripper-org/io-core).
