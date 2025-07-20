#include <Magick++.h>
#include <iostream>
#include <optional>
#include <filesystem>
#include <utility>
#include <format>

namespace {

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

// ===================== Command Line Parsing =====================

void helpMessage(){std::cout << "usage: ttypic [image-path]\n"; }

// NOLINTBEGIN
// Don't lint the pointer arithmetic involved with argv

std::optional<std::filesystem::path> getImageFromCmdlineArgs(const int argc,
                                                             char *argv[]) {
  if (argc <= 1) {
    return std::nullopt;
  }
  return argv[1] != nullptr ? std::make_optional(std::filesystem::path(
                                  std::string_view(argv[1])))
                            : std::nullopt;
}

void handleHelpFlagIfPresent(const int argc, char *argv[]) {
  if (argc <= 1 || argv[1] == nullptr) {
    return;
  }

  const std::string_view firstArg = argv[1];

  if (firstArg == "-h" || firstArg == "--help") {
      helpMessage();
      exit(0);
  }
}
// NOLINTEND

// ===== Image Processing ========================

Dimensions determineImageSize(const Magick::Geometry &imageSize,
                              const UInt maxWidth, const UInt maxHeight) {
  const Decimal scaleX = static_cast<Decimal>(maxWidth) /
                         static_cast<Decimal>(imageSize.width());
  const Decimal scaleY =
      static_cast<Decimal>(maxHeight) /
      (static_cast<Decimal>(imageSize.height()) * CHARACTER_ASPECT_RATIO);

  const Decimal scale = std::min(scaleX, scaleY);
  std::cout << "maxWidth = " << maxWidth << "\n";
  std::cout << "maxHeight = " << maxHeight << "\n";
  std::cout << "scale x: " << scaleX << "\n";
  std::cout << "scale y: " << scaleY << "\n";
  std::cout << "scale : " << scale << "\n";
  std::cout << "image width : " << imageSize.width() << "\n";
  std::cout << "image height : " << imageSize.height() << "\n";
  std::cout << "---" << "\n";
  return {
      .width =
          static_cast<UInt>(static_cast<Decimal>(imageSize.width()) * scale),
      .height = static_cast<UInt>(static_cast<Decimal>(imageSize.height()) *
                                  scale * CHARACTER_ASPECT_RATIO),
  };
}

void printImageArt(const std::filesystem::path &imagePath, const UInt termWidth,
                   const UInt termHeight) {
  const Magick::Image image(imagePath.c_str());
  const Dimensions targetDim =
      determineImageSize(image.size(), termWidth, termHeight);
  // const Dimensions targetDim = { .width = termWidth, .height = termHeight };

  std::cout << "targetDim width " << targetDim.width << "\n";
  std::cout << "targetDim height " << targetDim.height << "\n";

  for (UInt j = 0; j < targetDim.height; j++) {
    for (UInt i = 0; i < targetDim.width; i++) {
      const Decimal percentX =
          static_cast<Decimal>(i) / static_cast<Decimal>(termWidth);
      const Decimal percentY =
          static_cast<Decimal>(j) / static_cast<Decimal>(termHeight);

      const auto imgX =
          static_cast<UInt>(percentX * static_cast<Decimal>(image.size().width()));
      const auto imgY =
          static_cast<UInt>(percentY * static_cast<Decimal>(image.size().height()));

      const Magick::Color color = image.pixelColor(imgX, imgY);
      constexpr double MAGICK_QUANTUM_RANGE = 65535.0;
      const auto red =
          static_cast<uint8_t>(color.quantumRed() * 255 / MAGICK_QUANTUM_RANGE);
      const auto green = static_cast<uint8_t>(color.quantumGreen() * 255 /
                                              MAGICK_QUANTUM_RANGE);
      const auto blue = static_cast<uint8_t>(color.quantumBlue() * 255 /
                                             MAGICK_QUANTUM_RANGE);

      // Good Glyphs
      // - █
      // - 8
      // - #
      constexpr std::string_view GLYPH = "#";
      const std::string colorText =
          std::format("\x1b[38;2;{};{};{}m{}\x1b[0m", red, green, blue, GLYPH);

      std::cout << colorText;
    }
    std::cout << "\n";
  }
}

} // namespace

auto main(int argc, char *argv[]) -> int {
  handleHelpFlagIfPresent(argc, argv);

  const std::optional<std::filesystem::path> content =
      getImageFromCmdlineArgs(argc, argv);

  if (!content.has_value()) {
    helpMessage();
    exit(0);
  }

  const std::filesystem::path &path = content.value();
  if (!std::filesystem::exists(path)) {
    std::cout << path.c_str() << " does not exist!\n";
    exit(0);
  }

  // TODO
  // - cmdline args:
  //   + width and height
  //   + default should infer from terminal size
  //   + glyph to use for image
  // - graceful handle non images
  // - size config

  try {
    Magick::InitializeMagick(*argv);

    constexpr UInt WIDTH = 160 / 2 ;
    constexpr UInt HEIGHT = 45 / 2 ;
    printImageArt(path, WIDTH, HEIGHT);
  } catch (std::exception &e) {
    std::cout << "Ran into exception! " << e.what() << "\n";
  }

  return 0;
}
