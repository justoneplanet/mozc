// Copyright 2010-2012, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <string>

#include "base/util.h"
#include "config/config_handler.h"
#include "config/config.pb.h"
#include "converter/segments.h"
#include "converter/character_form_manager.h"
#include "rewriter/variants_rewriter.h"
#include "testing/base/public/gunit.h"

DECLARE_string(test_tmpdir);

namespace mozc {

class VariantsRewriterTest : public testing::Test {
 protected:
  virtual void SetUp() {
    Reset();
  }

  virtual void TearDown() {
    Reset();
  }

  void Reset() {
    Util::SetUserProfileDirectory(FLAGS_test_tmpdir);
    config::Config config;
    config::ConfigHandler::GetDefaultConfig(&config);
    config::ConfigHandler::SetConfig(config);
    CharacterFormManager::GetCharacterFormManager()->SetDefaultRule();
    CharacterFormManager::GetCharacterFormManager()->ClearHistory();
  }

  void InitSegmentsForAlphabetRewrite(const string &value,
                                      Segments *segments) const {
    Segment *segment = segments->push_back_segment();
    CHECK(segment);
    segment->set_key(value);
    Segment::Candidate *candidate = segment->add_candidate();
    CHECK(candidate);
    candidate->Init();
    candidate->key = value;
    candidate->content_key = value;
    candidate->value = value;
    candidate->content_value = value;
  }
};

TEST_F(VariantsRewriterTest, RewriteTest) {
  VariantsRewriter rewriter;
  Segments segments;

  Segment *seg = segments.push_back_segment();

  {
    Segment::Candidate *candidate = seg->add_candidate();
    candidate->Init();
    // "あいう"
    candidate->value = "\xe3\x81\x82\xe3\x81\x84\xe3\x81\x86";
    // "あいう"
    candidate->content_value = "\xe3\x81\x82\xe3\x81\x84\xe3\x81\x86";
    EXPECT_FALSE(rewriter.Rewrite(&segments));
    seg->clear_candidates();
  }

  {
    Segment::Candidate *candidate = seg->add_candidate();
    candidate->Init();
    candidate->value = "012";
    candidate->content_value = "012";
    CharacterFormManager::GetCharacterFormManager()->
      SetCharacterForm("012", config::Config::FULL_WIDTH);

    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(2, seg->candidates_size());
    // "０１２"
    EXPECT_EQ("\xef\xbc\x90\xef\xbc\x91\xef\xbc\x92", seg->candidate(0).value);
    // "０１２"
    EXPECT_EQ("\xef\xbc\x90\xef\xbc\x91\xef\xbc\x92",
              seg->candidate(0).content_value);
    EXPECT_EQ("012", seg->candidate(1).value);
    EXPECT_EQ("012", seg->candidate(1).content_value);
    seg->clear_candidates();
  }

  {
    Segment::Candidate *candidate = seg->add_candidate();
    candidate->Init();
    candidate->value = "012";
    candidate->content_value = "012";
    candidate->attributes |= Segment::Candidate::NO_VARIANTS_EXPANSION;
    CharacterFormManager::GetCharacterFormManager()->
      SetCharacterForm("012", config::Config::FULL_WIDTH);

    EXPECT_FALSE(rewriter.Rewrite(&segments));
    EXPECT_EQ(1, seg->candidates_size());
    seg->clear_candidates();
  }

  {
    Segment::Candidate *candidate = seg->add_candidate();
    candidate->Init();
    candidate->value = "Google";
    candidate->content_value = "Google";
    CharacterFormManager::GetCharacterFormManager()->
      SetCharacterForm("abc", config::Config::FULL_WIDTH);

    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(2, seg->candidates_size());
    // "Ｇｏｏｇｌｅ"
    EXPECT_EQ("\xef\xbc\xa7\xef\xbd\x8f\xef\xbd\x8f\xef\xbd\x87\xef\xbd\x8c\xef"
              "\xbd\x85", seg->candidate(0).value);
    // "Ｇｏｏｇｌｅ"
    EXPECT_EQ("\xef\xbc\xa7\xef\xbd\x8f\xef\xbd\x8f\xef\xbd\x87\xef\xbd\x8c\xef"
              "\xbd\x85", seg->candidate(0).content_value);
    EXPECT_EQ("Google", seg->candidate(1).value);
    EXPECT_EQ("Google", seg->candidate(1).content_value);
    seg->clear_candidates();
  }

  {
    Segment::Candidate *candidate = seg->add_candidate();
    candidate->Init();
    candidate->value = "@";
    candidate->content_value = "@";
    CharacterFormManager::GetCharacterFormManager()->
      SetCharacterForm("@", config::Config::FULL_WIDTH);

    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(2, seg->candidates_size());
    // "＠"
    EXPECT_EQ("\xef\xbc\xa0", seg->candidate(0).value);
    // "＠"
    EXPECT_EQ("\xef\xbc\xa0", seg->candidate(0).content_value);
    EXPECT_EQ("@", seg->candidate(1).value);
    EXPECT_EQ("@", seg->candidate(1).content_value);
    seg->clear_candidates();
  }

  {
    Segment::Candidate *candidate = seg->add_candidate();
    candidate->Init();
    // "グーグル"
    candidate->value = "\xe3\x82\xb0\xe3\x83\xbc\xe3\x82\xb0\xe3\x83\xab";
    // "グーグル"
    candidate->content_value = "\xe3\x82\xb0\xe3\x83\xbc\xe3\x82\xb0\xe3\x83"
                               "\xab";
    CharacterFormManager::GetCharacterFormManager()->
      // "アイウ"
      SetCharacterForm("\xe3\x82\xa2\xe3\x82\xa4\xe3\x82\xa6",
                       config::Config::FULL_WIDTH);

    EXPECT_FALSE(rewriter.Rewrite(&segments));
    seg->clear_candidates();
  }

  {
     Segment::Candidate *candidate = seg->add_candidate();
     candidate->Init();
     // "グーグル"
     candidate->value = "\xe3\x82\xb0\xe3\x83\xbc\xe3\x82\xb0\xe3\x83\xab";
     // "グーグル"
     candidate->content_value = "\xe3\x82\xb0\xe3\x83\xbc\xe3\x82\xb0\xe3\x83"
                                "\xab";
     CharacterFormManager::GetCharacterFormManager()->
         // "アイウ"
         AddConversionRule("\xe3\x82\xa2\xe3\x82\xa4\xe3\x82\xa6",
                           config::Config::HALF_WIDTH);

     EXPECT_TRUE(rewriter.Rewrite(&segments));
     EXPECT_EQ(2, seg->candidates_size());
     // "ｸﾞｰｸﾞﾙ"
     EXPECT_EQ("\xef\xbd\xb8\xef\xbe\x9e\xef\xbd\xb0\xef\xbd\xb8\xef\xbe\x9e"
               "\xef\xbe\x99", seg->candidate(0).value);
     // "ｸﾞｰｸﾞﾙ"
     EXPECT_EQ("\xef\xbd\xb8\xef\xbe\x9e\xef\xbd\xb0\xef\xbd\xb8\xef\xbe\x9e"
               "\xef\xbe\x99", seg->candidate(0).content_value);
     // "グーグル"
     EXPECT_EQ("\xe3\x82\xb0\xe3\x83\xbc\xe3\x82\xb0\xe3\x83\xab",
               seg->candidate(1).value);
     // "グーグル"
     EXPECT_EQ("\xe3\x82\xb0\xe3\x83\xbc\xe3\x82\xb0\xe3\x83\xab",
               seg->candidate(1).content_value);
     seg->clear_candidates();
  }
}

TEST_F(VariantsRewriterTest, RewriteTestManyCandidates) {
  VariantsRewriter rewriter;
  Segments segments;
  Segment *seg = segments.push_back_segment();

  config::Config config;
  config::ConfigHandler::GetDefaultConfig(&config);
  config::ConfigHandler::SetConfig(config);

  {
    for (int i = 0; i < 10; ++i) {
      Segment::Candidate *candidate1 = seg->add_candidate();
      candidate1->Init();
      candidate1->value = Util::SimpleItoa(i);
      candidate1->content_value = Util::SimpleItoa(i);
      Segment::Candidate *candidate2 = seg->add_candidate();
      candidate2->Init();
      // "ぐーぐる"
      candidate2->content_key =
          "\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B";
      candidate2->key =
          "\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B";
      candidate2->value =
          "\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B";
      candidate2->content_value =
          "\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B";
    }

    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(30, seg->candidates_size());

    for (int i = 0; i < 10; ++i) {
      EXPECT_EQ(Util::SimpleItoa(i), seg->candidate(3 * i + 1).value);
      EXPECT_EQ(Util::SimpleItoa(i), seg->candidate(3 * i + 1).content_value);
      string full_width;
      Util::HalfWidthToFullWidth(seg->candidate(3 * i + 1).value, &full_width);
      EXPECT_EQ(full_width, seg->candidate(3 * i).value);
      EXPECT_EQ(full_width, seg->candidate(3 * i).content_value);
      // "ぐーぐる"
      EXPECT_EQ("\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B",
                seg->candidate(3 * i + 2).value);
      EXPECT_EQ("\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B",
                seg->candidate(3 * i + 2).content_value);
    }
  }

  {
    seg->Clear();

    for (int i = 0; i < 10; ++i) {
      Segment::Candidate *candidate1 = seg->add_candidate();
      candidate1->Init();
      // "ぐーぐる"
      candidate1->content_key =
          "\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B";
      candidate1->key =
          "\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B";
      candidate1->value =
          "\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B";
      candidate1->content_value =
          "\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B";
      Segment::Candidate *candidate2 = seg->add_candidate();
      candidate2->Init();
      candidate2->value = Util::SimpleItoa(i);
      candidate2->content_value = Util::SimpleItoa(i);
    }

    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(30, seg->candidates_size());

    for (int i = 0; i < 10; ++i) {
      EXPECT_EQ(Util::SimpleItoa(i), seg->candidate(3 * i + 2).value);
      EXPECT_EQ(Util::SimpleItoa(i), seg->candidate(3 * i + 2).content_value);
      string full_width;
      Util::HalfWidthToFullWidth(seg->candidate(3 * i + 2).value, &full_width);
      EXPECT_EQ(full_width, seg->candidate(3 * i + 1).value);
      EXPECT_EQ(full_width, seg->candidate(3 * i + 1).content_value);
      // "ぐーぐる"
      EXPECT_EQ("\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B",
                seg->candidate(3 * i).value);
      EXPECT_EQ("\xE3\x81\x90\xE3\x83\xBC\xE3\x81\x90\xE3\x82\x8B",
                seg->candidate(3 * i).content_value);
    }
  }
}

TEST_F(VariantsRewriterTest, SetDescriptionForCandidate) {
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "HalfASCII";
    candidate.content_value = candidate.value;
    candidate.content_key = "halfascii";
    VariantsRewriter::SetDescriptionForCandidate(&candidate);
    // "[半] アルファベット"
    EXPECT_EQ("\x5b\xe5\x8d\x8a\x5d\x20\xe3\x82\xa2\xe3\x83\xab\xe3\x83\x95\xe3"
              "\x82\xa1\xe3\x83\x99\xe3\x83\x83\xe3\x83\x88",
              candidate.description);
  }
  // containing symbols
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "Half ASCII";
    candidate.content_value = candidate.value;
    candidate.content_key = "half ascii";
    VariantsRewriter::SetDescriptionForCandidate(&candidate);
    // "[半] アルファベット"
    EXPECT_EQ("\x5b\xe5\x8d\x8a\x5d\x20\xe3\x82\xa2\xe3\x83\xab\xe3\x83\x95\xe3"
              "\x82\xa1\xe3\x83\x99\xe3\x83\x83\xe3\x83\x88",
              candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "Half!ASCII!";
    candidate.content_value = candidate.value;
    candidate.content_key = "half!ascii!";
    VariantsRewriter::SetDescriptionForCandidate(&candidate);
    // "[半] アルファベット"
    EXPECT_EQ("\x5b\xe5\x8d\x8a\x5d\x20\xe3\x82\xa2\xe3\x83\xab\xe3\x83\x95\xe3"
              "\x82\xa1\xe3\x83\x99\xe3\x83\x83\xe3\x83\x88",
              candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "CD-ROM";
    candidate.content_value = candidate.value;
    // "しーでぃーろむ"
    candidate.content_key =
        "\xe3\x81\x97\xe3\x83\xbc\xe3\x81\xa7\xe3\x81\x83\xe3"
        "\x83\xbc\xe3\x82\x8d\xe3\x82\x80";
    VariantsRewriter::SetDescriptionForCandidate(&candidate);
    // "[半] アルファベット"
    EXPECT_EQ("\x5b\xe5\x8d\x8a\x5d\x20\xe3\x82\xa2\xe3\x83\xab\xe3\x83\x95\xe3"
              "\x82\xa1\xe3\x83\x99\xe3\x83\x83\xe3\x83\x88",
              candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    // "コギト・エルゴ・スム"
    candidate.value = "\xe3\x82\xb3\xe3\x82\xae\xe3\x83\x88\xe3\x83\xbb\xe3\x82"
        "\xa8\xe3\x83\xab\xe3\x82\xb4\xe3\x83\xbb\xe3\x82\xb9\xe3\x83\xa0";
    candidate.content_value = candidate.value;
    // "こぎとえるごすむ"
    candidate.content_key =
        "\xe3\x81\x93\xe3\x81\x8e\xe3\x81\xa8\xe3\x81\x88\xe3\x82\x8b\xe3\x81"
        "\x94\xe3\x81\x99\xe3\x82\x80";
    VariantsRewriter::SetDescriptionForCandidate(&candidate);
    // "[全] カタカナ"
    EXPECT_EQ(
        "\x5b\xe5\x85\xa8\x5d\x20\xe3\x82\xab\xe3\x82\xbf\xe3\x82\xab"
        "\xe3\x83\x8a",
        candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "!@#";
    candidate.content_value = candidate.value;
    candidate.content_key = "!@#";
    VariantsRewriter::SetDescriptionForCandidate(&candidate);
    // "[半]"
    EXPECT_EQ("\x5b\xe5\x8d\x8a\x5d", candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    // "「ＡＢＣ」"
    candidate.value = "\xe3\x80\x8c\xef\xbc\xa1\xef\xbc\xa2\xef\xbc\xa3\xe3"
                      "\x80\x8d";
    candidate.content_value = candidate.value;
    candidate.content_key = "[ABC]";
    VariantsRewriter::SetDescriptionForCandidate(&candidate);
    // "[全] アルファベット"
    EXPECT_EQ("\x5b\xe5\x85\xa8\x5d\x20\xe3\x82\xa2\xe3\x83\xab\xe3\x83\x95"
              "\xe3\x82\xa1\xe3\x83\x99\xe3\x83\x83\xe3\x83\x88",
              candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    // "草彅剛"
    candidate.value = "\xE8\x8D\x89\xE5\xBD\x85\xE5\x89\x9B";
    candidate.content_value = candidate.value;
    // "くさなぎつよし"
    candidate.content_key = "\xE3\x81\x8F\xE3\x81\x95\xE3\x81\xAA"
        "\xE3\x81\x8E\xE3\x81\xA4\xE3\x82\x88\xE3\x81\x97";
    VariantsRewriter::SetDescriptionForCandidate(&candidate);
    // "<機種依存文字>"
    EXPECT_EQ("<\xE6\xA9\x9F\xE7\xA8\xAE\xE4\xBE\x9D"
              "\xE5\xAD\x98\xE6\x96\x87\xE5\xAD\x97>", candidate.description);
  }
}

TEST_F(VariantsRewriterTest, SetDescriptionForTransliteration) {
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "HalfASCII";
    candidate.content_value = candidate.value;
    candidate.content_key = "halfascii";
    VariantsRewriter::SetDescriptionForTransliteration(&candidate);
    // "[半] アルファベット"
    EXPECT_EQ("\x5b\xe5\x8d\x8a\x5d\x20\xe3\x82\xa2\xe3\x83\xab\xe3\x83\x95\xe3"
              "\x82\xa1\xe3\x83\x99\xe3\x83\x83\xe3\x83\x88",
              candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "!@#";
    candidate.content_value = candidate.value;
    candidate.content_key = "!@#";
    VariantsRewriter::SetDescriptionForTransliteration(&candidate);
    // "[半]"
    EXPECT_EQ("\x5b\xe5\x8d\x8a\x5d", candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    // "「ＡＢＣ」"
    candidate.value = "\xe3\x80\x8c\xef\xbc\xa1\xef\xbc\xa2\xef\xbc\xa3\xe3"
                      "\x80\x8d";
    candidate.content_value = candidate.value;
    candidate.content_key = "[ABC]";
    VariantsRewriter::SetDescriptionForTransliteration(&candidate);
    // "[全] アルファベット"
    EXPECT_EQ("\x5b\xe5\x85\xa8\x5d\x20\xe3\x82\xa2\xe3\x83\xab\xe3\x83\x95"
              "\xe3\x82\xa1\xe3\x83\x99\xe3\x83\x83\xe3\x83\x88",
              candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    // "草彅剛"
    candidate.value = "\xE8\x8D\x89\xE5\xBD\x85\xE5\x89\x9B";
    candidate.content_value = candidate.value;
    // "くさなぎつよし"
    candidate.content_key = "\xE3\x81\x8F\xE3\x81\x95\xE3\x81\xAA"
        "\xE3\x81\x8E\xE3\x81\xA4\xE3\x82\x88\xE3\x81\x97";
    VariantsRewriter::SetDescriptionForTransliteration(&candidate);
    // "<機種依存文字>"
    EXPECT_EQ("<\xE6\xA9\x9F\xE7\xA8\xAE\xE4\xBE\x9D"
              "\xE5\xAD\x98\xE6\x96\x87\xE5\xAD\x97>", candidate.description);
  }
}

TEST_F(VariantsRewriterTest, SetDescriptionForPrediction) {
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "HalfASCII";
    candidate.content_value = candidate.value;
    candidate.content_key = "halfascii";
    VariantsRewriter::SetDescriptionForPrediction(&candidate);
    EXPECT_EQ("", candidate.description);
  }
  // containing symbols
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "Half ASCII";
    candidate.content_value = candidate.value;
    candidate.content_key = "half ascii";
    VariantsRewriter::SetDescriptionForPrediction(&candidate);
    EXPECT_EQ("", candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "Half!ASCII!";
    candidate.content_value = candidate.value;
    candidate.content_key = "half!ascii!";
    VariantsRewriter::SetDescriptionForPrediction(&candidate);
    EXPECT_EQ("", candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "CD-ROM";
    candidate.content_value = candidate.value;
    // "しーでぃーろむ"
    candidate.content_key =
        "\xe3\x81\x97\xe3\x83\xbc\xe3\x81\xa7\xe3\x81\x83\xe3"
        "\x83\xbc\xe3\x82\x8d\xe3\x82\x80";
    VariantsRewriter::SetDescriptionForPrediction(&candidate);
    EXPECT_EQ("", candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    candidate.value = "!@#";
    candidate.content_value = candidate.value;
    candidate.content_key = "!@#";
    VariantsRewriter::SetDescriptionForPrediction(&candidate);
    EXPECT_EQ("", candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    // "「ＡＢＣ」"
    candidate.value = "\xe3\x80\x8c\xef\xbc\xa1\xef\xbc\xa2\xef\xbc\xa3\xe3"
                      "\x80\x8d";
    candidate.content_value = candidate.value;
    candidate.content_key = "[ABC]";
    VariantsRewriter::SetDescriptionForPrediction(&candidate);
    // "[全] アルファベット"
    EXPECT_EQ("", candidate.description);
  }
  {
    Segment::Candidate candidate;
    candidate.Init();
    // "草彅剛"
    candidate.value = "\xE8\x8D\x89\xE5\xBD\x85\xE5\x89\x9B";
    candidate.content_value = candidate.value;
    // "くさなぎつよし"
    candidate.content_key = "\xE3\x81\x8F\xE3\x81\x95\xE3\x81\xAA"
        "\xE3\x81\x8E\xE3\x81\xA4\xE3\x82\x88\xE3\x81\x97";
    VariantsRewriter::SetDescriptionForPrediction(&candidate);
    // "<機種依存文字>"
    EXPECT_EQ("<\xE6\xA9\x9F\xE7\xA8\xAE\xE4\xBE\x9D"
              "\xE5\xAD\x98\xE6\x96\x87\xE5\xAD\x97>", candidate.description);
  }
}

TEST_F(VariantsRewriterTest, RewriteForConversion) {
  CharacterFormManager *character_form_manager =
      CharacterFormManager::GetCharacterFormManager();
  VariantsRewriter rewriter;
  {
    Segments segments;
    segments.set_request_type(Segments::CONVERSION);
    {
      Segment *segment = segments.push_back_segment();
      segment->set_key("abc");
      Segment::Candidate *candidate = segment->add_candidate();
      candidate->Init();
      candidate->key = "abc";
      candidate->content_key = "abc";
      candidate->value = "abc";
      candidate->content_value = "abc";
    }
    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(1, segments.segments_size());
    EXPECT_EQ(2, segments.segment(0).candidates_size());

    EXPECT_EQ(config::Config::FULL_WIDTH,
              character_form_manager->GetConversionCharacterForm("abc"));

    // "ａｂｃ"
    EXPECT_EQ("\xef\xbd\x81\xef\xbd\x82\xef\xbd\x83",
              segments.segment(0).candidate(0).value);
    EXPECT_EQ("abc", segments.segment(0).candidate(1).value);
  }
  {
    character_form_manager->SetCharacterForm("abc", config::Config::HALF_WIDTH);
    Segments segments;
    segments.set_request_type(Segments::CONVERSION);
    {
      Segment *segment = segments.push_back_segment();
      segment->set_key("abc");
      Segment::Candidate *candidate = segment->add_candidate();
      candidate->Init();
      candidate->key = "abc";
      candidate->content_key = "abc";
      candidate->value = "abc";
      candidate->content_value = "abc";
    }
    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(1, segments.segments_size());
    EXPECT_EQ(2, segments.segment(0).candidates_size());

    EXPECT_EQ(config::Config::HALF_WIDTH,
              character_form_manager->GetConversionCharacterForm("abc"));

    EXPECT_EQ("abc", segments.segment(0).candidate(0).value);
    // "ａｂｃ"
    EXPECT_EQ("\xef\xbd\x81\xef\xbd\x82\xef\xbd\x83",
              segments.segment(0).candidate(1).value);
  }
}

TEST_F(VariantsRewriterTest, RewriteForPrediction) {
  CharacterFormManager *character_form_manager =
      CharacterFormManager::GetCharacterFormManager();
  VariantsRewriter rewriter;
  {
    Segments segments;
    segments.set_request_type(Segments::PREDICTION);
    InitSegmentsForAlphabetRewrite("abc", &segments);
    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(1, segments.segments_size());
    EXPECT_EQ(2, segments.segment(0).candidates_size());

    EXPECT_EQ(config::Config::FULL_WIDTH,
              character_form_manager->GetConversionCharacterForm("abc"));

    // "ａｂｃ"
    EXPECT_EQ("\xef\xbd\x81\xef\xbd\x82\xef\xbd\x83",
              segments.segment(0).candidate(0).value);
    EXPECT_EQ("abc", segments.segment(0).candidate(1).value);
  }
  {
    character_form_manager->SetCharacterForm("abc", config::Config::HALF_WIDTH);
    Segments segments;
    segments.set_request_type(Segments::PREDICTION);
    InitSegmentsForAlphabetRewrite("abc", &segments);
    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(1, segments.segments_size());
    EXPECT_EQ(2, segments.segment(0).candidates_size());

    EXPECT_EQ(config::Config::HALF_WIDTH,
              character_form_manager->GetConversionCharacterForm("abc"));

    EXPECT_EQ("abc", segments.segment(0).candidate(0).value);
    // "ａｂｃ"
    EXPECT_EQ("\xef\xbd\x81\xef\xbd\x82\xef\xbd\x83",
              segments.segment(0).candidate(1).value);
  }
}

TEST_F(VariantsRewriterTest, RewriteForSuggestion) {
  CharacterFormManager *character_form_manager =
      CharacterFormManager::GetCharacterFormManager();
  VariantsRewriter rewriter;
  {
    Segments segments;
    segments.set_request_type(Segments::SUGGESTION);
    InitSegmentsForAlphabetRewrite("abc", &segments);
    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(1, segments.segments_size());
    EXPECT_EQ(1, segments.segment(0).candidates_size());

    EXPECT_EQ(config::Config::FULL_WIDTH,
              character_form_manager->GetConversionCharacterForm("abc"));

    // "ａｂｃ"
    EXPECT_EQ("\xef\xbd\x81\xef\xbd\x82\xef\xbd\x83",
              segments.segment(0).candidate(0).value);
  }
  {
    character_form_manager->SetCharacterForm("abc", config::Config::HALF_WIDTH);
    Segments segments;
    segments.set_request_type(Segments::SUGGESTION);
    InitSegmentsForAlphabetRewrite("abc", &segments);
    EXPECT_TRUE(rewriter.Rewrite(&segments));
    EXPECT_EQ(1, segments.segments_size());
    EXPECT_EQ(1, segments.segment(0).candidates_size());

    EXPECT_EQ(config::Config::HALF_WIDTH,
              character_form_manager->GetConversionCharacterForm("abc"));

    EXPECT_EQ("abc", segments.segment(0).candidate(0).value);
  }
}

TEST_F(VariantsRewriterTest, Capability) {
  VariantsRewriter rewriter;
  EXPECT_EQ(RewriterInterface::ALL, rewriter.capability());
}
}  // namespace mozc