#include <gtest/gtest.h>
#include "memory/SharedPtr.h"
#include <string>

// --- Helper for Lifecycle Tracking ---
struct Tracker {
    static int constructed_count;
    static int destructed_count;
    int value;

    Tracker(int v) : value(v) { constructed_count++; }
    ~Tracker() { destructed_count++; }

    static void reset_counts() {
        constructed_count = 0;
        destructed_count = 0;
    }
};

int Tracker::constructed_count = 0;
int Tracker::destructed_count = 0;

class SharedPtrTest : public ::testing::Test {
protected:
    void SetUp() override {
        Tracker::reset_counts();
    }
};

// 1. Basic Construction
TEST_F(SharedPtrTest, DefaultConstructor) {
    My::SharedPtr<int> p;
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.use_count(), 0);
    EXPECT_FALSE(p);
}

TEST_F(SharedPtrTest, RawPointerConstructor) {
    {
        My::SharedPtr<Tracker> p(new Tracker(10));
        EXPECT_EQ(Tracker::constructed_count, 1);
        EXPECT_EQ(p->value, 10);
        EXPECT_EQ(p.use_count(), 1);
    }
    EXPECT_EQ(Tracker::destructed_count, 1);
}

// 2. Copy Semantics (Reference Counting)
TEST_F(SharedPtrTest, CopyConstructor) {
    My::SharedPtr<Tracker> p1(new Tracker(100));
    {
        My::SharedPtr<Tracker> p2 = p1; // Copy
        EXPECT_EQ(p1.use_count(), 2);
        EXPECT_EQ(p2.use_count(), 2);
        EXPECT_EQ(p1.get(), p2.get());
    } // p2 dies here

    EXPECT_EQ(p1.use_count(), 1);
    EXPECT_EQ(Tracker::destructed_count, 0); // Object still alive
}

TEST_F(SharedPtrTest, CopyAssignment) {
    My::SharedPtr<Tracker> p1(new Tracker(1));
    My::SharedPtr<Tracker> p2(new Tracker(2));

    p1 = p2;
    // Tracker(1) should be destroyed.
    // Tracker(2) ref count should be 2.

    EXPECT_EQ(Tracker::destructed_count, 1);
    EXPECT_EQ(p1->value, 2);
    EXPECT_EQ(p1.use_count(), 2);
    EXPECT_EQ(p2.use_count(), 2);
}

TEST_F(SharedPtrTest, SelfAssignment) {
    My::SharedPtr<Tracker> p(new Tracker(5));
    p = p; // Should do nothing
    EXPECT_EQ(p.use_count(), 1);
    EXPECT_EQ(Tracker::destructed_count, 0);
}

// 3. Move Semantics
TEST_F(SharedPtrTest, MoveConstructor) {
    My::SharedPtr<Tracker> p1(new Tracker(10));
    My::SharedPtr<Tracker> p2(std::move(p1));

    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_EQ(p1.use_count(), 0);

    EXPECT_NE(p2.get(), nullptr);
    EXPECT_EQ(p2.use_count(), 1);
}

TEST_F(SharedPtrTest, MoveAssignment) {
    My::SharedPtr<Tracker> p1(new Tracker(10));
    My::SharedPtr<Tracker> p2(new Tracker(20));

    p2 = std::move(p1);
    // Tracker(20) destroyed.
    // p2 owns Tracker(10).
    // p1 is empty.

    EXPECT_EQ(Tracker::destructed_count, 1);
    EXPECT_EQ(p2->value, 10);
    EXPECT_EQ(p2.use_count(), 1);
}

// 4. Reset
TEST_F(SharedPtrTest, Reset) {
    My::SharedPtr<Tracker> p(new Tracker(1));
    p.reset();
    EXPECT_EQ(Tracker::destructed_count, 1);
    EXPECT_EQ(p.use_count(), 0);
    EXPECT_EQ(p.get(), nullptr);
}

TEST_F(SharedPtrTest, ResetWithPtr) {
    My::SharedPtr<Tracker> p(new Tracker(1));
    p.reset(new Tracker(2));
    EXPECT_EQ(Tracker::destructed_count, 1);
    EXPECT_EQ(p->value, 2);
    EXPECT_EQ(p.use_count(), 1);
}

// 5. make_shared
TEST_F(SharedPtrTest, MakeShared) {
    auto p = My::make_shared<Tracker>(999);

    EXPECT_EQ(Tracker::constructed_count, 1);
    EXPECT_EQ(p->value, 999);
    EXPECT_EQ(p.use_count(), 1);

    {
        auto p2 = p;
        EXPECT_EQ(p.use_count(), 2);
    }
    EXPECT_EQ(p.use_count(), 1);
}

// 6. Independent Control Blocks
TEST_F(SharedPtrTest, IndependentPointers) {
    // Two independent pointers to DIFFERENT objects
    My::SharedPtr<Tracker> p1(new Tracker(1));
    My::SharedPtr<Tracker> p2(new Tracker(2));

    EXPECT_EQ(p1.use_count(), 1);
    EXPECT_EQ(p2.use_count(), 1);
}
