#include "gtest/gtest.h"

#include <opentimelineio/serializableCollection.h>
#include <copentimelineio/errorStatus.h>
#include <copentimelineio/serializableCollection.h>
#include <copentimelineio/serializableObject.h>
#include <pthread.h>

class OTIOMultithreadingTests : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    static void* test_bash_retainers1(void* sc)
    {
        pthread_mutex_t lock;
        pthread_mutex_lock(&lock);

        OTIO_NS::SerializableCollection* serializableCollection =
            reinterpret_cast<OTIO_NS::SerializableCollection*>(sc);

        OTIO_NS::SerializableObject* so = serializableCollection->children()[0];

        int total = 0;
        for(size_t i = 0; i < 1024 * 10; i++)
        {
            OTIO_NS::SerializableObject::Retainer<> r(so);
            if(r.value) { total++; }
        }

        pthread_mutex_unlock(&lock);
        int* totalReturn = new int(total);
        return totalReturn;
    }
};

TEST_F(OTIOMultithreadingTests, Test1)
{
    OTIOSerializableObject* child = SerializableObject_create();
    OTIO_RETAIN(child);
    SerializableObjectVector* childrenVector = SerializableObjectVector_create();
    SerializableObjectVector_push_back(childrenVector, child);

    SerializableCollection* sc = SerializableCollection_create(NULL, childrenVector, NULL);
    OTIO_RETAIN(sc);

    pthread_t tid[5];

    for(int i = 0; i < 5; ++i)
    {
        pthread_create(&tid[i], NULL, test_bash_retainers1, sc);
    }

    for(int i = 0; i < 5; ++i)
    {
        pthread_join(tid[i], NULL);
    }

    SerializableObjectVector_destroy(childrenVector);
    childrenVector = NULL;
    OTIO_RELEASE(child);
    child = NULL;
    OTIO_RELEASE(sc);
    sc = NULL;
}
