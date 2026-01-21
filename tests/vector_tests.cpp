#include <gtest/gtest.h>
#include "memory/Vector.h"
#include <string>

using namespace My;

// Helper to track Constructor/Destructor calls (Detects leaks)
struct Tracker {
    static int constructions;
    static int destructions;
    int val;

    Tracker(int v) : val(v) { constructions++; }
    Tracker(const Tracker& o) : val(o.val) { constructions++; }
    Tracker(Tracker&& o) noexcept : val(o.val) { constructions++; o.val = -1; }
    ~Tracker() { destructions++; }

    static void reset() { constructions = 0; destructions = 0; }
};

int Tracker::constructions = 0;
int Tracker::destructions = 0;


// --- BASIC FUNCTIONALITY ---

TEST(VectorTest, DefaultConstruction) {
    Vector<int> v;
    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.capacity(), 0);
    EXPECT_TRUE(v.empty());
}

TEST(VectorTest, PushBackBasic) {
    Vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[2], 3);
}


// --- MEMORY & GROWTH ---

TEST(VectorTest, GeometricGrowth) {
    Vector<int> v;
    v.reserve(2);
    size_t cap_start = v.capacity();

    //// Fill capacity
    v.push_back(1);
    v.push_back(2);
    EXPECT_EQ(v.capacity(), cap_start);

    // Trigger resize
    v.push_back(3);
    EXPECT_GT(v.capacity(), cap_start);
    EXPECT_GE(v.capacity(), 3);
}

TEST(VectorTest, ReserveDoesNotShrink) {
    Vector<int> v;
    v.reserve(10);
    size_t cap = v.capacity();

    v.reserve(5); // Should be ignored
    EXPECT_EQ(v.capacity(), cap);
}

// --- POINTER STABILITY (HFT CRITICAL) ---

TEST(VectorTest, PointerStabilityWithinCapacity) {
    Vector<int> v;
    v.reserve(100);
    v.push_back(1);

    int* ptr = &v[0];

    for(int i=0; i<90; ++i) v.push_back(i);

    // If reserve works, this pointer is still valid
    EXPECT_EQ(&v[0], ptr);
}

// --- RESOURCE MANAGEMENT ---

TEST(VectorTest, DestructorCleansUp) {
    Tracker::reset();
    {
        Vector<Tracker> v;
        v.push_back(Tracker(1));
        v.push_back(Tracker(2));
        v.push_back(Tracker(3));
    } // v dies here

    // 3 objects created, 3 objects destroyed
    // (Note: push_back might create temporaries, so we check equality, not absolute numbers)
    EXPECT_EQ(Tracker::constructions, Tracker::destructions);
}

TEST(VectorTest, ClearRetainsCapacity) {
    Vector<int> v;
    v.reserve(10);
    v.push_back(1);
    v.push_back(2);

    size_t cap = v.capacity();
    v.clear();

    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.capacity(), cap); // Memory should NOT be freed
}

TEST(VectorTest, EmplaceBackEfficiency) {
    struct Complex {
        int x, y;
        Complex(int a, int b) : x(a), y(b) {}
    };

    Vector<Complex> v;
    // Should construct directly in place: new (&addr) Complex(10, 20)
    Complex& ref = v.emplace_back(10, 20);

    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v[0].x, 10);
    EXPECT_EQ(ref.x, 10);
}

/*
// --- MOVE SEMANTICS ---

TEST(VectorTest, MoveConstructor) {
    Vector<int> v1;
    v1.push_back(100);
    int* data_ptr = v1.data();

    Vector<int> v2(std::move(v1));

    // v2 stole the data
    EXPECT_EQ(v2.data(), data_ptr);
    EXPECT_EQ(v2.size(), 1);

    // v1 is empty
    EXPECT_EQ(v1.data(), nullptr);
    EXPECT_EQ(v1.size(), 0);
}
*/
