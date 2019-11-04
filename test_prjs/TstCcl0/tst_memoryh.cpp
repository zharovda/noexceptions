#include "tst_memoryh.h"

#include "..\..\ccl\memory.h"
#include "..\..\ccl\asserts.h"

using namespace ccl::memory;


bool test_shared_ptr_1() noexcept;

long test_memory_h() noexcept
{
    test_shared_ptr_1();

    return 1;
}



bool test_shared_ptr_1() noexcept
{
    shared_ptr<int> empty;
    assert(empty.use_count() == 0);

  //  shared_ptr<int> ptr_1(new int(11));;

    return true;
  
}