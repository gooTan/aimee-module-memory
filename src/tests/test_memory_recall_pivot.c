/* test_memory_recall_pivot.c: topic-pivot detection for per-turn
 * recall. Pure function — no DB state required. Exercises the
 * stopword filter, the min-length filter, the empty-side short-
 * circuits, and the threshold boundary.
 *
 * See docs/proposals/done/personal-agent-phase-2-per-turn-recall.md. */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "aimee.h"
#include "memory.h"

static void expect_pivot(const char *prev, const char *cur, int expected, const char *tag)
{
   int got = memory_recall_topic_pivot(prev, cur, 0);
   if (got != expected)
   {
      fprintf(stderr, "pivot check '%s': expected=%d got=%d prev=%s cur=%s\n", tag, expected, got,
              prev ? prev : "(null)", cur ? cur : "(null)");
      assert(0);
   }
}

int main(void)
{
   printf("memory_recall_topic_pivot: ");

   /* Empty / NULL sides are never a pivot — session-start and
    * signal-free turns both fall through to the regular recall path. */
   expect_pivot(NULL, "write the migration", 0, "null_prev");
   expect_pivot("", "write the migration", 0, "empty_prev");
   expect_pivot("write the migration", NULL, 0, "null_cur");
   expect_pivot("write the migration", "", 0, "empty_cur");

   /* Same content → full overlap → not a pivot. */
   expect_pivot("write the migration script", "write the migration script", 0, "identical");

   /* Clearly related turns keep substantial token overlap — not a
    * pivot even when wording changes. "write the migration script" ∩
    * "rerun the migration script with the rollback flag" keeps
    * {migration, script} from a 2-token prev set → Jaccard 1.0. */
   expect_pivot("write the migration script", "rerun the migration script with the rollback flag",
                0, "topic_continues");

   /* Clear pivot: completely different domains, no shared non-stopword
    * tokens. */
   expect_pivot("write the migration script", "how do I tune the embedding model", 1,
                "topic_pivot_migration_to_embedding");

   /* Pivot across three unrelated subjects (check the primitive is
    * commutative on argument order). */
   expect_pivot("debugging the vector sidecar lifecycle", "plan the onboarding checklist", 1,
                "topic_pivot_vector_to_onboarding");
   expect_pivot("plan the onboarding checklist", "debugging the vector sidecar lifecycle", 1,
                "topic_pivot_reversed");

   /* Stopwords and short tokens (< 3 chars) don't count toward
    * overlap — an all-stopword "current" turn has zero significant
    * tokens and short-circuits to "no pivot" so we don't churn on a
    * vague reply like "ok do it". */
   expect_pivot("rerank the results", "do it", 0, "short_words_stopwords");
   expect_pivot("do it", "rerank the results", 0, "short_words_stopwords_reversed");

   /* Case-insensitive match: upper vs lower should overlap. */
   expect_pivot("Rerank the Results", "rerank the results", 0, "case_insensitive");

   /* Punctuation doesn't split tokens into fragments that survive the
    * min-length filter spuriously. "vector/sidecar" is two tokens
    * {vector, sidecar}, same as "vector sidecar". */
   expect_pivot("vector/sidecar lifecycle", "vector sidecar lifecycle", 0, "punctuation");

   /* Boundary: a single shared token on each side (prev={foo}, cur={foo})
    * is Jaccard 1.0 and clearly not a pivot. */
   expect_pivot("vector", "vector", 0, "single_token_overlap");

   /* One shared keyword is not enough to prevent a pivot when the
    * rest of the turn is unrelated — with the default 0.15 threshold,
    * Jaccard of ~1/11 falls well below and a pivot is the right call. */
   expect_pivot("schedule the migration for tomorrow night with staging",
                "migration scheduling is not related to the frontend rewrite at all", 1,
                "single_shared_token_still_pivots");

   /* Explicit threshold lets callers tune the decision. "abc def"
    * vs "abc ghi" share one of three union tokens → Jaccard 0.333.
    * A loose 0.2 threshold accepts the overlap; a tight 0.5 declares
    * a pivot. */
   assert(memory_recall_topic_pivot("abc def", "abc ghi", 0.2) == 0);
   assert(memory_recall_topic_pivot("abc def", "abc ghi", 0.5) == 1);
   /* Extremely tight threshold forces pivot whenever anything differs. */
   assert(memory_recall_topic_pivot("abc def", "abc ghi", 0.99) == 1);

   /* Duplicates in the same turn don't inflate the set (Jaccard is
    * set-valued, not multiset-valued). "script script script" should
    * behave the same as "script". */
   expect_pivot("migration script script script", "migration script", 0, "dedup_within_turn");

   printf("all tests passed\n");
   return 0;
}
