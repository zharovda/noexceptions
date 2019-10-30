#pragma once

/*
TODO: убрать заголовки! добавить необходимые traits
*/

#include <utility.h>

//common code library
using ccl = std; //для отладки можно заменить собственные traits и тп на std

// TODO: доопределить


#ifndef NT_SUCCESS
typedef long NTSTATUS;
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)
#endif

#ifndef STATUS_INSUFFICIENT_RESOURCES
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)     // ntsubauth
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
#endif

#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)    // winnt
#endif






//============================================================================================================
//	базовые шаблоны выделения и освобождения памяти 
//============================================================================================================

//------------------------------------------------------------------------------------------------------------
//	простой шаблон менеджера памяти (реализация выделения и освобождения)
//
//	Примечание: данный шаблон - пример реализации менеджера памяти ресурсов. В простейшем случае используется
//				new/delete с последующем, после выделения памяти, вызовом функции инициализации объекта 
//				(placement new) 
//
//------------------------------------------------------------------------------------------------------------
template <typename T>
struct DefaultMemManager
{
	DefaultMemManager() = default;

	NTSTATUS allocate(__out T** ptr);
	void deallocate(__in T* ptr);


	using type = T;
};



template <typename T>
NTSTATUS 
DefaultMemManager<T>::allocate(
	__out T** ptr)
{
	// контракт
	assert(nullptr != ptr);	
	assert(nullptr == *ptr);

	// реализация
	*ptr = static_cast<T*>(new unsigned char[sizeof(T)]);
	if (nullptr == *ptr) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	return STATUS_SUCCESS;
}


template <typename T>
void
DefaultMemManager<T>::deallocate(
	__in T* ptr)
{
	// контракт
	//assert(nullptr != ptr);

	// реализация
	if (ptr) 
	{
		ptr->~T();
		delete [] reinterpret_cast<unsigned char*>(ptr);
	}
}



//============================================================================================================
//	базовый класс счетчика ссылок (виртуальный)
//
//	Примечания: 
//		* weak pointer не предусмотрен
//		* базовый класс используется для реализации специфического счетчика ссылок
//============================================================================================================
class RefCounterBase {
public:

	RefCounterBase(RefCounterBase&& other) :
		uses_(ccl::move(other.uses_))
	{}

	RefCounterBase() = default;
	virtual ~RefCounterBase() = default;

	// increment use count
	void incref() { _InterlockedIncrement(reinterpret_cast<volatile long *>(&uses_)); }

	// decrement use count
	void decref() {
		// destroy managed resource, decrement weak reference count
		if (_InterlockedDecrement(reinterpret_cast<volatile long *>(&uses_)) == 0) {
			destroy();
			delete_this();	//в оригинале нужно уменьшить число ссылок, но в упрощенном виде (без weak pointer) этого достаточно
		}
	}

	size_t use_count() { return uses_; }

private:
	virtual void destroy() = 0;
	virtual void delete_this() = 0;

private:
	volatile size_t uses_ = 1;
	//volatile size_t weak_ = 1;	//в данной реализации weak и uses эквивалентны
};



//============================================================================================================
//	шаблон счетчика ссылок (кастомные менеджеры памяти для счетчика и ресурса)
//
//	Примечание: шаблон реализация которого предусматривает использование кастомного менеджера памяти для 
//				создания и удаления счетчика ссылок. 
//				
//
//============================================================================================================
template<
	class _Resource,			// тип хранимого объекта
	class _DxResource,			// класс удаления объекта
	class _DxCounter>			// класс удаления счетчика ссылок
class RefCounter
	: public RefCounterBase
{
	RefCounter() = delete;
	RefCounter(const RefCounter&) = delete;
	RefCounter& operator=(const RefCounter&) = delete;

public:
	
	RefCounter(RefCounter&& other) : 
		RefCounterBase(ccl::move(other)),
		ptr_(ccl::move(other.ptr_)),
		dx_res_(ccl::move(other.dx_res_)),
		dx_counter_(ccl::move(other.dx_counter_))
	{}

	explicit RefCounter(_Resource* res_ptr, _DxResource dx_res, _DxCounter dx_counter) :
		RefCounterBase(), 
		dx_res_(ccl::move(dx_res)), 
		dx_counter_(ccl::move(dx_counter)),
		ptr_(res_ptr)
	{}

	~RefCounter() {}

	// destroy managed resource
	virtual void destroy() override 
	{ 
		if (ptr_) 
		{ 
			dx_res_.deallocate(ptr_);
			ptr_ = nullptr;
		} 
	}

	virtual void delete_this() override 
	{ 
		_DxCounter dx(ccl::move(dx_counter_));
		dx.deallocate(this);
	}

private:
	_Resource*	ptr_ = nullptr;

	_DxResource dx_res_;
	_DxCounter  dx_counter_;
};


//============================================================================================================
//	шаблон менеджера памяти по умолчанию для создания/удаления счетчика ссылок
//
//============================================================================================================
template <
	class _Ty,
	class _DxResource 
>
struct DefaultResCounterMM
{
	using TDxCounter = RefCounter<_Ty, _DxResource, DefaultResCounterMM>;

	DefaultResCounterMM() = default;

	NTSTATUS allocate(TDxCounter** ptr)
	{
		*ptr = reinterpret_cast<TDxCounter*>(new unsigned char[sizeof(TDxCounter)]);
		if (nullptr == *ptr) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		return STATUS_SUCCESS;
	}

	void deallocate(TDxCounter* ptr)
	{
		ptr->~TDxCounter();

		unsigned char* real_ptr = reinterpret_cast<unsigned char*>(ptr);
		if (nullptr != real_ptr) {
			delete[] real_ptr;
		}
	}

	using type = TDxCounter;
};



//============================================================================================================
//	шаблон базового указателя с контролирующим блоком (счетчик ссылок)
//============================================================================================================
template<class _Ty>
class shared_ptr;


template<class _Ty>
class _Ptr_base
{	
	using element_type = std::remove_extent_t<_Ty>;
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
		// метод ТОЛЬКО для конструкторов
		_Ptr = _Right._Ptr;
		_Rep = _Right._Rep;

		_Right._Ptr = nullptr;
		_Right._Rep = nullptr;
	}

	template<class _Ty2>
	void _Copy_construct_from(const shared_ptr<_Ty2>& _Other)
	{	// implement shared_ptr's (converting) copy ctor
		// метод ТОЛЬКО для конструкторов
		if (_Other._Rep)
		{
			_Other._Rep->incref();
		}

		_Ptr = _Other._Ptr;
		_Rep = _Other._Rep;
	}

	void _Decref()
	{	// decrement reference count
		if (_Rep)
		{
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
//	служебная функция создания и инициализации счетчика ссылок
//============================================================================================================
template<
	class _Ty,
	class _TyDeleter,	// класс реализующий функцию удаления ресурса 
	class _MmCounter	// класс создания и удаления простого счетчика 
>
NTSTATUS
_init(
	__in  _Ty* ptr,
	__in  _TyDeleter res_dtor,
	__in  _MmCounter counter_mm,
	__out typename _MmCounter::type** ppcounter)
{
	using counter_mm_type = _MmCounter;
	using counter_type = typename counter_mm_type::type;
	using res_deleter_type = _TyDeleter;

	counter_type* p_counter = nullptr;

	NTSTATUS st = counter_mm.allocate(&p_counter);
	if (!NT_SUCCESS(st)) {
		return st;
	}

	new (p_counter) counter_type(ptr, ccl::move(res_dtor), ccl::move(counter_mm));

	*ppcounter = p_counter;
	p_counter = nullptr;

	return st;
}

//============================================================================================================
//	шаблон shared_ptr
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

	typedef void(__stdcall * TOwnedObjectReleaser)(res_type_pointer);

   
public:

	template<
		class _TPtr,
		class _MmTy,
		class _MmCounter 	// класс создания и удаления простого счетчика 
	>
	friend
	NTSTATUS
		make_shared(__in _TPtr* ptr, __inout shared_ptr<_TPtr>& empty_container) noexcept;
	


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
	_NODISCARD _Ty2& operator*() const noexcept
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
	class _TPtr,
	class _MmTy = DefaultMemManager<_TPtr>,
	class _MmCounter = DefaultResCounterMM<_TPtr, _MmTy> 	// класс создания и удаления простого счетчика 
>
NTSTATUS
make_shared(__in _TPtr* ptr, __inout shared_ptr<_TPtr>& empty_container) noexcept
{
	NTSTATUS st = STATUS_UNSUCCESSFUL;

	_MmTy res_dtor;
	_MmCounter counter_mm;
	typename _MmCounter::type* counter_obj = nullptr;

	st = _init<_TPtr, _MmTy, _MmCounter>(ptr, ccl::move(res_dtor), ccl::move(counter_mm), &counter_obj);
	if (!NT_SUCCESS(st)) {
		return st;
	}

	empty_container._setpd(ptr, counter_obj);

	return STATUS_SUCCESS;
}
