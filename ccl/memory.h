//============================================================================================================
//	memory manipulation wrappers and smart pointer definitions
//============================================================================================================

#ifndef __MEMORY_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_
#define __MEMORY_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_

#include ".\kbasic.h"
#include ".\asserts.h"

#include ".\traits.h"
#include ".\utility.h"

#ifdef __KERNEL_MODE
#include <wdm.h>
#define ccl_interlocked_increment InterlockedIncrement
#define ccl_interlocked_decrement InterlockedDecrement
#else   // __KERNEL_MODE
#include <intrin0.h>
#define ccl_interlocked_increment _InterlockedIncrement
#define ccl_interlocked_decrement _InterlockedDecrement
#endif  // __KERNEL_MODE


namespace ccl
{

namespace memory
{
    
//============================================================================================================
//	basic memory managment wrappers
//============================================================================================================


//------------------------------------------------------------------------------------------------------------
//  default allocators/deleters
//------------------------------------------------------------------------------------------------------------
namespace mm
{
namespace regular
{
template <
    typename _Tx
>
struct _default_allocator
{
    constexpr _default_allocator() noexcept = default;

    _Tx* operator()() const noexcept { // delete a pointer
        return new _Tx();
    }

    using type = _Tx;
};


template <
    typename _Tx
>
struct _default_deleter
{
    constexpr _default_deleter() noexcept = default;

    void operator()(_Tx* _Ptr) const noexcept { // delete a pointer
        static_assert(0 < sizeof(_Tx), "can't delete an incomplete type");
        delete _Ptr;
    }

    using type = _Tx;
};
} // namespace regular


namespace placement 
{
template <
    typename _Tx
>
struct _default_allocator
{
    constexpr _default_allocator() noexcept = default;

    _Tx* operator()() const noexcept { // delete a pointer
        return reinterpret_cast<_Tx*>(new unsigned char[sizeof(_Tx)]);
    }

    using type = _Tx;
};


template <
    typename _Tx
>
struct _default_deleter
{
    constexpr _default_deleter() noexcept = default;

    void operator()(_Tx* _Ptr) const noexcept { // delete a pointer
        static_assert(0 < sizeof(_Tx), "can't delete an incomplete type");

        _Ptr->~_Tx();
        delete[] reinterpret_cast<unsigned char*>(_Ptr);
    }

    using type = _Tx;
};
} // namespace placement 

} // namespace mm



//============================================================================================================
//  base reference counter class definition
//  Realisation of smart pointer reference counter should be derived by using this class.
//
//  This realisation does not support weak pointers. 
//============================================================================================================
class RefCounterBase 
{

public:

	RefCounterBase(RefCounterBase&& other) :
		uses_(ccl::move(other.uses_))
	{}

	RefCounterBase() = default;
	virtual ~RefCounterBase() = default;

	// increment use count
	void incref() { ccl_interlocked_increment(reinterpret_cast<volatile long *>(&uses_)); }

	// decrement use count
	void decref() {
		// destroy managed resource, decrement weak reference count
		if (ccl_interlocked_decrement(reinterpret_cast<volatile long *>(&uses_)) == 0) {
			destroy();
			delete_this();	//в оригинале нужно уменьшить число ссылок, но в упрощенном виде (без weak pointer) этого достаточно
		}
	}

	size_t use_count() { return uses_; }

private:
	virtual void destroy() = 0;
	virtual void delete_this() = 0;

	volatile size_t uses_ = 1;
	//volatile size_t weak_ = 1;	//as historical reference
};


//============================================================================================================
//  reference counter template with custom memory managers 
//============================================================================================================
template<
	class _TResource,			// resource type which lifecircle is controlled by reference counter
	class _DxResource,			// resource deleter class
	class _DxCounter>			// reference counter deleter object
class RefCounter
	: public RefCounterBase
{
	RefCounter() = delete;
	RefCounter(const RefCounter&) = delete;
	RefCounter& operator=(const RefCounter&) = delete;

public:
	
	RefCounter(RefCounter&& other) : 
		RefCounterBase(ccl::move(other)),
		dx_res_(ccl::move(other.dx_res_)),
		dx_counter_(ccl::move(other.dx_counter_)),
        ptr_(other.ptr_)
	{}

	explicit RefCounter(_TResource* res_ptr, _DxResource dx_res, _DxCounter dx_counter) :
		RefCounterBase(), 
		dx_res_(ccl::move(dx_res)), 
		dx_counter_(ccl::move(dx_counter)),
		ptr_(res_ptr)
	{}

	~RefCounter() {}

	// destroy managed resource
	void destroy() final 
	{ 
		if (ptr_) 
		{ 
			dx_res_()(ptr_);
			ptr_ = nullptr;
		} 
	}

	void delete_this() final 
	{ 
		_DxCounter dx(ccl::move(dx_counter_));
		dx(this);
	}

private:
	_TResource*	ptr_ = nullptr;

	_DxResource dx_res_;
	_DxCounter  dx_counter_;
};



//============================================================================================================
//	base pointer template, and shared_ptr forward declaration
//============================================================================================================
template<class _Ty> class shared_ptr;


template<class _Ty>
class _Ptr_base
{	
	using element_type = ccl::remove_extent_t<_Ty>;
	using ref_counter_type = RefCounterBase;

public:

	size_t use_count() const noexcept
	{	// return use count
		return (_Rep ? _Rep->use_count() : 0);
	}

	_Ptr_base(const _Ptr_base&) = delete;
	_Ptr_base& operator=(const _Ptr_base&) = delete;

protected:
	element_type* get() const noexcept
	{	// return pointer to resource
		return (_Ptr);
	}

	constexpr _Ptr_base() noexcept = default;

	~_Ptr_base() = default;

	template<class _Ty2>
	void _Move_construct_from(_Ptr_base<_Ty2>&& _Right)
	{	// implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
		_Ptr = _Right._Ptr;
		_Rep = _Right._Rep;

		_Right._Ptr = nullptr;
		_Right._Rep = nullptr;
	}

	template<class _Ty2>
	void _Copy_construct_from(const shared_ptr<_Ty2>& _Other)
	{	// implement shared_ptr's (converting) copy ctor
		if (_Other._Rep){
			_Other._Rep->incref();
		}

		_Ptr = _Other._Ptr;
		_Rep = _Other._Rep;
	}

	void _Decref()
	{	// decrement reference count
		if (_Rep){
			_Rep->decref();
		}
	}

	void _Swap(_Ptr_base& _Right) noexcept
	{			
		// swap pointers
		ccl::swap(_Ptr, _Right._Ptr);
		ccl::swap(_Rep, _Right._Rep);
	}

	void _Set_ptr_rep(element_type* _Other_ptr, ref_counter_type* _Other_rep)
	{	// take new resource
		_Ptr = _Other_ptr;
		_Rep = _Other_rep;
	}


private:
	element_type* _Ptr{ nullptr };
	ref_counter_type* _Rep{ nullptr };
};


//============================================================================================================
//	template function that creates and initialize reference counter for resource
//============================================================================================================
template<
    class _Ty,
    class _TyDeleter,	    // resource deleter
    class _AlRefCounter,	// reference counter allocator
    class _DlRefCounter,	// reference counter deleter
>
ccl::kbasic::NtStatus
_init(
    __in  _Ty* ptr,
    __in  _TyDeleter res_dtor,
    __out typename _DlRefCounter::type** ppcounter)
{
    using counter_al_type = _AlRefCounter;
    using counter_dl_type = _DlRefCounter;
    using counter_type = typename _DlRefCounter::type;

    using NtStatusVals = typename ccl::kbasic::NtStatusVals;
    

    counter_al_type al;
    counter_type* p_counter = al();
    if (nullptr == p_counter) {
        return ccl::kbasic::status(NtStatusVals::NtStatusInsufficientResources);
    }

    counter_dl_type dx_counter;
    new (p_counter) counter_type(ptr, ccl::move(res_dtor), ccl::move(dx_counter));

    *ppcounter = p_counter;
    p_counter = nullptr;

    return ccl::kbasic::status(NtStatusVals::NtStatusSuccess);
}

//============================================================================================================
//	shared_ptr
//============================================================================================================
template<
	class _Ty
>					
class shared_ptr
	: public _Ptr_base<_Ty>
{	// class for reference counted resource management
private:

	using res_type = _Ty;
	using res_type_pointer = res_type*;

	using _Mybase = _Ptr_base<res_type>;
   
public:

    // shared_ptr class friend function for initialization and check it result without exceptions
    template<class _Tx, class _TyDeleter, class _AlRefCounter, class _DlRefCounter>
    friend
    ccl::kbasic::NtStatus
        make_shared(__in _Tx* ptr, __inout shared_ptr<_Tx>& empty_container) noexcept;


	constexpr shared_ptr() noexcept
	{	// construct empty shared_ptr
	}

	constexpr shared_ptr(nullptr_t) noexcept
	{	// construct empty shared_ptr
	}


	shared_ptr(const shared_ptr& _Other) noexcept
	{	// construct shared_ptr object that owns same resource as _Other
		this->_Copy_construct_from(_Other);
	}


	shared_ptr(shared_ptr&& _Right) noexcept
	{	// construct shared_ptr object that takes resource from _Right
		this->_Move_construct_from(ccl::move(_Right));
	}

	~shared_ptr() noexcept
	{	// release resource
		this->_Decref();
	}


	shared_ptr& operator=(const shared_ptr& _Right) noexcept
	{	// assign shared ownership of resource owned by _Right
		shared_ptr(_Right).swap(*this);
		return (*this);
	}


	void swap(shared_ptr& _Other) noexcept
	{	// swap pointers
		this->_Swap(_Other);
	}

	shared_ptr& operator=(shared_ptr&& _Right) noexcept
	{	// take resource from _Right
		shared_ptr(ccl::move(_Right)).swap(*this);
		return (*this);
	}

	template<class _Ty2 = _Ty>
	_Ty2& operator*() const noexcept
	{	// return reference to resource
		return (*get());
	}

	void reset() noexcept
	{	// release resource and convert to empty shared_ptr object
		shared_ptr().swap(*this);
	}
			
	template<class _Ux>
	void reset(_Ux * _Px)
	{	// release, take ownership of _Px
		shared_ptr(_Px).swap(*this);
	}


	using _Mybase::get;


	template<class _Ty2 = _Ty>
	_NODISCARD _Ty2 * operator->() const noexcept
	{	// return pointer to resource
		return (get());
	}

	_NODISCARD bool unique() const noexcept
	{	// return true if no other shared_ptr object owns this resource
		return (this->use_count() == 1);
	}

	explicit operator bool() const noexcept
	{	// test if shared_ptr object owns a resource
		return (get() != nullptr);
	}


private:

	void _setpd(res_type_pointer ptr, RefCounterBase* ref_counter)
	{
		this->_Set_ptr_rep(ptr, ref_counter);
	}
};




template<
    class _Tx,
    class _TyDeleter = mm::regular::_default_deleter<_Tx>,
    class _AlRefCounter = mm::regular::_default_deleter<_Tx>,	// reference counter allocator
    class _DlRefCounter = mm::regular::_default_deleter<_Tx>	// reference counter deleter
>
ccl::kbasic::NtStatus
make_shared(__in _Tx* ptr, __inout shared_ptr<_Tx>& empty_container) noexcept
{
    using ccl::kbasic::status;
    using ccl::kbasic::ntsuccess;
    using ccl::kbasic::NtStatus;
    using ccl::kbasic::NtStatusVals;

    using ref_counter_del_t = typename _DlRefCounter::type;

    NtStatus st = status(NtStatusVals::NtStatusInvalidParameter);

    _TyDeleter res_dtor;

    ref_counter_del_t* counter_obj = nullptr;

    st = _init<_Tx, _TyDeleter, _AlRefCounter, _DlRefCounter>(ptr, ccl::move(res_dtor), &counter_obj);
    if (!ntsuccess(st)) {
        return st;
    }

    empty_container._setpd(ptr, counter_obj);
    return st;
}

} // namespace memory

} // namespace ccl


#endif // __MEMORY_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_