/*
 * Copyright 2015-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/json.h>

#include <folly/Benchmark.h>
#include <folly/Conv.h>
#include <folly/Range.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/GTest.h>

using folly::dynamic;
using folly::parseJson;
using folly::toJson;

constexpr folly::StringPiece kLargeAsciiString =
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk"
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk"
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk"
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk"
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk"
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk"
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk"
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk"
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk"
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk"
    "akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk";

constexpr folly::StringPiece kLargeNonAsciiString =
    "qwerty \xc2\x80 \xef\xbf\xbf poiuy"
    "qwerty \xc2\x80 \xef\xbf\xbf poiuy"
    "qwerty \xc2\x80 \xef\xbf\xbf poiuy"
    "qwerty \xc2\x80 \xef\xbf\xbf poiuy"
    "qwerty \xc2\x80 \xef\xbf\xbf poiuy"
    "qwerty \xc2\x80 \xef\xbf\xbf poiuy"
    "qwerty \xc2\x80 \xef\xbf\xbf poiuy"
    "qwerty \xc2\x80 \xef\xbf\xbf poiuy"
    "qwerty \xc2\x80 \xef\xbf\xbf poiuy"
    "qwerty \xc2\x80 \xef\xbf\xbf poiuy";

constexpr folly::StringPiece kLargeAsciiStringWithSpecialChars =
    "<script>foo%@bar.com</script>"
    "<script>foo%@bar.com</script>"
    "<script>foo%@bar.com</script>"
    "<script>foo%@bar.com</script>"
    "<script>foo%@bar.com</script>"
    "<script>foo%@bar.com</script>"
    "<script>foo%@bar.com</script>";

TEST(Json, StripComments) {
  auto testStr = folly::stripLeftMargin(R"JSON(
    {
      // comment
      "test": "foo", // comment
      "test2": "foo // bar", // more comments
      /*
      "test3": "baz"
      */
      "test4": "foo /* bar", /* comment */
      "te//": "foo",
      "te/*": "bar",
      "\\\"": "\\" /* comment */
    }
  )JSON");
  auto expectedStr = folly::stripLeftMargin(R"JSON(
    {
        "test": "foo",   "test2": "foo // bar",   
      "test4": "foo /* bar", 
      "te//": "foo",
      "te/*": "bar",
      "\\\"": "\\" 
    }
  )JSON");

  EXPECT_EQ(expectedStr, folly::json::stripComments(testStr));
}

BENCHMARK(jsonSerialize, iters) {
  const dynamic obj = kLargeNonAsciiString;

  folly::json::serialization_opts opts;
  for (size_t i = 0; i < iters; ++i) {
    folly::json::serialize(obj, opts);
  }
}

BENCHMARK(jsonSerializeWithNonAsciiEncoding, iters) {
  const dynamic obj = kLargeNonAsciiString;

  folly::json::serialization_opts opts;
  opts.encode_non_ascii = true;

  for (size_t i = 0; i < iters; ++i) {
    folly::json::serialize(obj, opts);
  }
}

BENCHMARK(jsonSerializeWithUtf8Validation, iters) {
  const dynamic obj = kLargeNonAsciiString;

  folly::json::serialization_opts opts;
  opts.validate_utf8 = true;

  for (size_t i = 0; i < iters; ++i) {
    folly::json::serialize(obj, opts);
  }
}

BENCHMARK(jsonSerializeAsciiWithUtf8Validation, iters) {
  const dynamic obj = kLargeAsciiString;

  folly::json::serialization_opts opts;
  opts.validate_utf8 = true;

  for (size_t i = 0; i < iters; ++i) {
    folly::json::serialize(obj, opts);
  }
}

BENCHMARK(jsonSerializeWithExtraUnicodeEscapes, iters) {
  const dynamic obj = kLargeAsciiStringWithSpecialChars;

  folly::json::serialization_opts opts;
  opts.extra_ascii_to_escape_bitmap =
      folly::json::buildExtraAsciiToEscapeBitmap("<%@");

  for (size_t i = 0; i < iters; ++i) {
    folly::json::serialize(obj, opts);
  }
}

BENCHMARK(parseSmallStringWithUtf, iters) {
  for (size_t i = 0; i < iters << 4; ++i) {
    parseJson("\"I \\u2665 UTF-8 thjasdhkjh blah blah blah\"");
  }
}

BENCHMARK(parseNormalString, iters) {
  for (size_t i = 0; i < iters << 4; ++i) {
    parseJson("\"akjhfk jhkjlakjhfk jhkjlakjhfk jhkjl akjhfk\"");
  }
}

BENCHMARK(parseBigString, iters) {
  const auto json = folly::to<std::string>('"', kLargeAsciiString, '"');

  for (size_t i = 0; i < iters; ++i) {
    parseJson(json);
  }
}

BENCHMARK(toJson, iters) {
  dynamic something = parseJson(
      "{\"old_value\":40,\"changed\":true,\"opened\":false,\"foo\":[1,2,3,4,5,6]}");

  for (size_t i = 0; i < iters; i++) {
    toJson(something);
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (FLAGS_benchmark) {
    folly::runBenchmarks();
  }
  return RUN_ALL_TESTS();
}
