/**
 * @file
 */

#include "core/tests/AbstractTest.h"
#include "stock/ItemProvider.h"
#include "stock/Inventory.h"

namespace stock {

class AbstractStockTest: public core::AbstractTest {
protected:
	ItemProvider _provider;
	ItemData *_itemData1;
	ItemData *_itemData2;
	Inventory _inv;
	const uint8_t _containerId = 0u;
	const Container* _container;
	Item* _item1;
	Item* _item2;
public:
	virtual void SetUp() override {
		core::AbstractTest::SetUp();
		_itemData1 = new ItemData(1, ItemType::WEAPON);
		_itemData1->setSize(1, 1);
		ASSERT_TRUE(_provider.addItemData(_itemData1)) << "Could not add item to provider for id 1";
		ASSERT_EQ(_itemData1, _provider.getItemData(1)) << "Could not get item data for id 1";
		_itemData2 = new ItemData(2, ItemType::WEAPON);
		_itemData2->setSize(1, 1);
		ASSERT_TRUE(_provider.addItemData(_itemData2)) << "Could not add item to provider for id 2";
		ASSERT_EQ(_itemData1, _provider.getItemData(1)) << "Could not get item data for id 1";
		ASSERT_EQ(_itemData2, _provider.getItemData(2)) << "Could not get item data for id 2";
		ItemData* itemDataDup = new ItemData(1, ItemType::WEAPON);
		ASSERT_FALSE(_provider.addItemData(itemDataDup)) << "Added duplicated item id 1 to item provider";
		delete itemDataDup;

		_itemData1->setSize(1, 2);
		_itemData2->setSize(1, 1);

		ContainerShape shape;
		shape.addRect(0, 1, 1, 1);
		shape.addRect(1, 1, 4, 4);
		ASSERT_TRUE(_inv.initContainer(_containerId, shape));

		_container = _inv.container(_containerId);

		_item1 = _provider.createItem(_itemData1->id());
		_item2 = _provider.createItem(_itemData2->id());
		ASSERT_EQ(2, _item1->shape().size());
		ASSERT_EQ(1, _item2->shape().size());
	}

	virtual void TearDown() override {
		core::AbstractTest::TearDown();
		_provider.shutdown();
		_itemData1 = nullptr;
		_itemData2 = nullptr;

		delete _item1;
		delete _item2;
		_item1 = _item2 = nullptr;
	}
};

}
