#include "testsMain.h"

#include "compare.h"
#include "../engine/ecs/registry.h"
#include "../util/log.h"

TEST_CASE(idGeneration) {
	using namespace Engine;
	Registry8 registry;
	auto id1 = registry.create();
	auto id2 = registry.create();
	registry.destroy(id1);
	auto id3 = registry.create();
	
	ASSERT_TRUE(id3 == id1);
}

TEST_CASE(componentGeneration) {
	using namespace Engine;
	Registry8 registry;
	auto id = registry.create();

	struct A {};
	struct B {};

	registry.add<A>(id);
	registry.add<B>(id);
	registry.remove<A>(id);

	ASSERT_FALSE(registry.has<A>(id));
	ASSERT_TRUE(registry.has<B>(id));
}

TEST_CASE(componentAccess) {
	using namespace Engine;
	Registry8 registry;
	auto id = registry.create();

	struct A {
		int a;
		float b;

		A(int a, float b) : a(a), b(b) {}
	};

	registry.add<A>(id, 1, 1.2);
	ASSERT_TRUE(registry.get<A>(id)->a == 1);
	ASSERT_TOLERANT(registry.get<A>(id)->b == 1.2, 0.000001);
}


TEST_CASE(viewTest) {
	struct A {};
	struct B {};
	struct C {};

	using namespace Engine;
	Registry8 registry;
	auto id1 = registry.create();
	auto id2 = registry.create();
	auto id3 = registry.create();
	auto id4 = registry.create();
	auto id5 = registry.create();

	registry.add<A>(id1);
	registry.add<A>(id2);
	registry.add<A>(id3);
	registry.add<A>(id4);

	registry.add<B>(id1);
	registry.add<B>(id3);
	registry.add<B>(id5);

	registry.add<C>(id1);
	registry.add<C>(id2);
	registry.add<C>(id3);
	registry.add<C>(id4);
	registry.add<C>(id5);
	
	std::size_t count = 0;
	for (auto _ : registry.view<A, B, C>()) {
		count++;
	}

	ASSERT_TRUE(count == 2);
}

TEST_CASE(entityParent) {
	using namespace Engine;
	Registry8 registry;
	auto id1 = registry.create();
	auto parent = registry.create();

	ASSERT_TRUE(registry.getParent(id1) == registry.null_entity);

	id1 = registry.setParent(id1, parent);

	ASSERT_FALSE(registry.getParent(id1) == registry.null_entity);
}

TEST_CASE(getChildren) {
	using namespace Engine;
	Registry8 registry;
	auto id1 = registry.create();
	auto id2 = registry.create();
	auto parent = registry.create();

	id1 = registry.setParent(id1, parent);
	id2 = registry.setParent(id2, parent);

	std::size_t count = 0;
	for (auto _ : registry.getChildren(parent)) {
		count++;
	}

	ASSERT_TRUE(count == 2);
}