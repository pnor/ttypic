#include <Magick++.h>
#include <filesystem>
#include <format>
#include <iostream>
#include <optional>
#include <sys/ioctl.h>
#include <unistd.h>

#include "include/argparse.hpp"

namespace {

using SmallUInt = uint8_t;
using UInt = uint16_t;
using Decimal = float;

struct Dimensions {
  UInt width;
  UInt height;
};

/**
 * width:height of a character
 */
constexpr Decimal CHARACTER_ASPECT_RATIO = 0.5;
constexpr Decimal DEFAULT_TERMINAL_SCALE_FACTOR = 0.5;
constexpr UInt DEFAULT_WIDTH = 160;
constexpr UInt DEFAULT_HEIGHT = 22;

// ===================== Command Line Parsing =====================

void helpMessage() { std::cout << "usage: ttypic [image-path]\n"; }

std::filesystem::path
getImageFromCmdlineArgs(const argparse::ArgumentParser &program) {
  const auto &path = program.get<std::string>("image_path");
  return {path};
}

Dimensions getTerminalSize() {
  int cols = DEFAULT_WIDTH;
  int lines = DEFAULT_HEIGHT;
#ifdef TIOCGSIZE
  struct ttysize termSize{};
  ioctl(STDIN_FILENO, TIOCGSIZE, &termSize);
  cols = termSize.ts_cols;
  lines = termSize.ts_lines;
#elif defined(TIOCGWINSZ)
  struct winsize termSize{};
  // NOLINTNEXTLINE
  ioctl(STDIN_FILENO, TIOCGWINSZ, &termSize);
  cols = termSize.ws_col;
  lines = termSize.ws_row;
#endif /* TIOCGSIZE */
  return {.width = static_cast<UInt>(cols), .height = static_cast<UInt>(lines)};
}

Dimensions getDimensionsFromCmdline(const argparse::ArgumentParser &program) {
  const std::optional<UInt> width = program.present<int>("--width");
  const std::optional<UInt> height = program.present<int>("--height");

  if (width.has_value() && height.has_value()) {
    return Dimensions{.width = width.value(), .height = height.value()};
  }

  Dimensions terminalDimensions = getTerminalSize();
  terminalDimensions.width =
      static_cast<UInt>(static_cast<Decimal>(terminalDimensions.width) *
                        DEFAULT_TERMINAL_SCALE_FACTOR);
  terminalDimensions.height =
      static_cast<UInt>(static_cast<Decimal>(terminalDimensions.height) *
                        DEFAULT_TERMINAL_SCALE_FACTOR);
  return terminalDimensions;
}

// ===== Image Processing ========================

Dimensions determineImageSize(const Magick::Geometry &imageSize,
                              const UInt maxWidth, const UInt maxHeight) {
  const Decimal scaleX =
      static_cast<Decimal>(maxWidth) / static_cast<Decimal>(imageSize.width());
  const Decimal scaleY =
      static_cast<Decimal>(maxHeight) /
      (static_cast<Decimal>(imageSize.height()) * CHARACTER_ASPECT_RATIO);

  const Decimal scale = std::min(scaleX, scaleY);
  return {
      .width =
          static_cast<UInt>(static_cast<Decimal>(imageSize.width()) * scale),
      .height = static_cast<UInt>(static_cast<Decimal>(imageSize.height()) *
                                  scale * CHARACTER_ASPECT_RATIO),
  };
}

Magick::Image getImage(const std::filesystem::path &imagePath) {
  try {
    const Magick::Image image(imagePath.c_str());
    return image;
  } catch (const std::exception &exc) {
    std::cout << imagePath.c_str() << " is not a valid image!" << "\n";
    exit(1);
  }
}

void printImageArt(const std::filesystem::path &imagePath, const UInt termWidth,
                   const UInt termHeight, const std::string_view glyphs) {
  const Magick::Image image = getImage(imagePath);
  const Dimensions targetDim =
      determineImageSize(image.size(), termWidth, termHeight);

  SmallUInt glyphCount = 0;
  for (UInt j = 0; j < targetDim.height; j++) {
    for (UInt i = 0; i < targetDim.width; i++) {
      const Decimal percentX =
          static_cast<Decimal>(i) / static_cast<Decimal>(targetDim.width);
      const Decimal percentY =
          static_cast<Decimal>(j) / static_cast<Decimal>(targetDim.height);

      const auto imgX = static_cast<UInt>(
          percentX * static_cast<Decimal>(image.size().width()));
      const auto imgY = static_cast<UInt>(
          percentY * static_cast<Decimal>(image.size().height()));

      const Magick::Color color = image.pixelColor(imgX, imgY);
      constexpr double MAGICK_QUANTUM_RANGE = 65535.0;
      const auto red =
          static_cast<uint8_t>(color.quantumRed() * 255 / MAGICK_QUANTUM_RANGE);
      const auto green = static_cast<uint8_t>(color.quantumGreen() * 255 /
                                              MAGICK_QUANTUM_RANGE);
      const auto blue = static_cast<uint8_t>(color.quantumBlue() * 255 /
                                             MAGICK_QUANTUM_RANGE);

      const std::string colorText = std::format(
          "\x1b[38;2;{};{};{}m{}\x1b[0m", red, green, blue, glyphs[glyphCount]);

      std::cout << colorText;

      glyphCount = (glyphCount + 1) % glyphs.size();
    }
    std::cout << "\n";
  }
}

} // namespace

auto main(int argc, char *argv[]) -> int {
  argparse::ArgumentParser program("ttypic");
  program.add_description("Print a picture as ASCII art");

  program.add_argument("image_path").help("Path of image to display");

  program.add_argument("-w", "--width")
      .help("Width in characters of output image")
      .scan<'i', int>();

  program.add_argument("-h", "--height")
      .help("Height in characters of output image")
      .scan<'i', int>();

  program.add_argument("-g", "--glyph")
      .help("Characters to use to draw ascii art")
      .default_value("#");

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << "\n\n";
    std::cerr << program;
    return 1;
  }

  const std::optional<std::filesystem::path> content =
      getImageFromCmdlineArgs(program);

  if (!content.has_value()) {
    helpMessage();
    exit(0);
  }

  const std::filesystem::path &path = content.value();
  if (!std::filesystem::exists(path)) {
    std::cout << path.c_str() << " does not exist!\n";
    exit(0);
  }

  try {
    Magick::InitializeMagick(*argv);
    const Dimensions artSize = getDimensionsFromCmdline(program);
    const auto glyphs = program.get<std::string>("--glyph");
    if (glyphs.empty()) {
      std::cout << "Glyph must be a single non-empty character!" << "\n";
      exit(1);
    }
    printImageArt(path, artSize.width, artSize.height, glyphs);
  } catch (std::exception &e) {
    std::cout << "Ran into exception! " << e.what() << "\n";
  }

  return 0;
}
