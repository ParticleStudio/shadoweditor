#ifndef BEHAVIORTREE_ANY_HPP
#define BEHAVIORTREE_ANY_HPP

#include <new>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>

#if defined(PARTICLE)
#if !defined(__cpp_exceptions) and !defined(ANY_IMPL_NO_EXCEPTIONS) and !defined(ANY_IMPL_EXCEPTIONS)
#define ANY_IMPL_NO_EXCEPTIONS
#endif
#else
// you can opt-out of exceptions by definining ANY_IMPL_NO_EXCEPTIONS,
// but you must ensure not to cast badly when passing an `any' object to any_cast<T>(any)
#endif

#if defined(PARTICLE)
#if !defined(__cpp_rtti) and !defined(ANY_IMPL_NO_RTTI) and !defined(ANY_IMPL_RTTI)
#define ANY_IMPL_NO_RTTI
#endif
#else
// you can opt-out of RTTI by defining ANY_IMPL_NO_RTTI,
// in order to disable functions working with the typeid of a type
#endif


namespace linb {
template<typename T>
struct in_place_type_t {
    constexpr explicit in_place_type_t() noexcept = default;
};

#if defined(__cpp_variable_templates) or defined(_MSC_VER)
template<typename T>
constexpr in_place_type_t<T> in_place_type{};
#endif

class bad_any_cast: public std::bad_cast {
 public:
    const char *what() const noexcept override {
        return "bad any Cast";
    }
};

class any final {
 public:
    /// Constructs an object of Type any with an Empty state.
    any() noexcept: vtable(nullptr) {
    }

    /// Constructs an object of Type any with an equivalent state as other.
    any(const any &refRhs): vtable(refRhs.vtable) {
        if(!refRhs.Empty()) {
            refRhs.vtable->copy(refRhs.storage, this->storage);
        }
    }

    /// Constructs an object of Type any with a state equivalent to the original state of other.
    /// rhs is left in a valid but otherwise unspecified state.
    any(any &&refRhs) noexcept: vtable(refRhs.vtable) {
        if(!refRhs.Empty()) {
            refRhs.vtable->move(refRhs.storage, this->storage);
            refRhs.vtable = nullptr;
        }
    }

    /// Same effect as this->Clear().
    ~any() {
        this->clear();
    }

    /// Constructs an object of Type any that contains an object of Type T direct-initialized with std::forward<ValueType>(value).
    ///
    /// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
    /// This is because an `any` may be copy constructed into another `any` at any time, so a copy should always be allowed.
    template<typename ValueType, typename = typename std::enable_if<!std::is_same<typename std::decay<ValueType>::type, any>::value>::type>
    any(ValueType &&refValue) {
        static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value, "T shall satisfy the CopyConstructible requirements.");
        this->construct(std::forward<ValueType>(refValue));
    }

    template<typename ValueType, typename... Args>
    explicit any(in_place_type_t<ValueType>, Args &&...args) {
        this->emplace_construct<ValueType>(std::forward<Args>(args)...);
    }

    template<typename ValueType, typename U, typename... Args>
    explicit any(in_place_type_t<ValueType>, std::initializer_list<U> il, Args &&...args) {
        this->emplace_construct<ValueType>(il, std::forward<Args>(args)...);
    }
    /// Has the same effect as any(rhs).swap(*this). No effects if an exception is thrown.
    any &operator=(const any &refRhs) {
        any(refRhs).swap(*this);
        return *this;
    }

    /// Has the same effect as any(std::move(rhs)).swap(*this).
    ///
    /// The state of *this is equivalent to the original state of rhs and rhs is left in a valid
    /// but otherwise unspecified state.
    any &operator=(any &&refRhs) noexcept {
        std::move(refRhs).swap(*this);
        return *this;
    }

    /// Has the same effect as any(std::forward<ValueType>(value)).swap(*this). No effect if a exception is thrown.
    ///
    /// T shall satisfy the CopyConstructible requirements, otherwise the program is ill-formed.
    /// This is because an `any` may be copy constructed into another `any` at any time, so a copy should always be allowed.
    template<typename ValueType, typename = typename std::enable_if<!std::is_same<typename std::decay<ValueType>::type, any>::value>::type>
    any &operator=(ValueType &&refValue) {
        static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value, "T shall satisfy the CopyConstructible requirements.");
        any(std::forward<ValueType>(refValue)).swap(*this);
        return *this;
    }

    /// If not Empty, destroys the contained object.
    void clear() noexcept {
        if(!Empty()) {
            this->vtable->destroy(storage);
            this->vtable = nullptr;
        }
    }

    /// Returns true if *this has no contained object, otherwise false.
    bool Empty() const noexcept {
        return this->vtable == nullptr;
    }

#ifndef ANY_IMPL_NO_RTTI
    /// If *this has a contained object of Type T, typeid(T); otherwise typeid(void).
    const std::type_info &Type() const noexcept {
        return Empty() ? typeid(void) : this->vtable->type();
    }
#endif

    /// Exchange the states of *this and rhs.
    void swap(any &refRhs) noexcept {
        if(this->vtable != refRhs.vtable) {
            any tmp(std::move(refRhs));

            // move from *this to rhs.
            refRhs.vtable = this->vtable;
            if(this->vtable != nullptr) {
                this->vtable->move(this->storage, refRhs.storage);
                //this->vtable = nullptr; -- unneeded, see below
            }

            // move from tmp (previously rhs) to *this.
            this->vtable = tmp.vtable;
            if(tmp.vtable != nullptr) {
                tmp.vtable->move(tmp.storage, this->storage);
                tmp.vtable = nullptr;
            }
        } else {// same types
            if(this->vtable != nullptr)
                this->vtable->swap(this->storage, refRhs.storage);
        }
    }

 private:// Storage and Virtual Method Table
    union storage_union {
        using stack_storage_t = typename std::aligned_storage<2 * sizeof(void *), std::alignment_of<void *>::value>::type;

        void *dynamic;
        stack_storage_t stack;// 2 words for e.g. shared_ptr
    };

    /// Base VTable specification.
    struct vtable_type {
        // Note: The caller is responsible for doing .vtable = nullptr after destructful operations
        // such as destroy() and/or move().

#ifndef ANY_IMPL_NO_RTTI
        /// The Type of the object this vtable is for.
        const std::type_info &(*type)() noexcept;
#endif

        /// Destroys the object in the union.
        /// The state of the union after this call is unspecified, caller must ensure not to use src anymore.
        void (*destroy)(storage_union &) noexcept;

        /// Copies the **inner** content of the src union into the yet unitialized dest union.
        /// As such, both inner objects will have the same state, but on separate memory locations.
        void (*copy)(const storage_union &src, storage_union &dest);

        /// Moves the storage from src to the yet unitialized dest union.
        /// The state of src after this call is unspecified, caller must ensure not to use src anymore.
        void (*move)(storage_union &src, storage_union &dest) noexcept;

        /// Exchanges the storage between lhs and rhs.
        void (*swap)(storage_union &lhs, storage_union &rhs) noexcept;
    };

    /// VTable for dynamically allocated storage.
    template<typename T>
    struct vtable_dynamic {
#ifndef ANY_IMPL_NO_RTTI
        static const std::type_info &type() noexcept {
            return typeid(T);
        }
#endif

        static void destroy(storage_union &refStorage) noexcept {
            //assert(reinterpret_cast<T*>(storage.dynamic));
            delete reinterpret_cast<T *>(refStorage.dynamic);
        }

        static void copy(const storage_union &refSrc, storage_union &refDest) {
            refDest.dynamic = new T(*reinterpret_cast<const T *>(refSrc.dynamic));
        }

        static void move(storage_union &refSrc, storage_union &refDest) noexcept {
            refDest.dynamic = refSrc.dynamic;
            refSrc.dynamic = nullptr;
        }

        static void swap(storage_union &refLhs, storage_union &refRhs) noexcept {
            // just exchange the storage pointers.
            std::swap(refLhs.dynamic, refRhs.dynamic);
        }
    };

    /// VTable for stack allocated storage.
    template<typename T>
    struct vtable_stack {
#ifndef ANY_IMPL_NO_RTTI
        static const std::type_info &type() noexcept {
            return typeid(T);
        }
#endif

        static void destroy(storage_union &refStorage) noexcept {
            reinterpret_cast<T *>(&refStorage.stack)->~T();
        }

        static void copy(const storage_union &refSrc, storage_union &refDest) {
            new(&refDest.stack) T(reinterpret_cast<const T &>(refSrc.stack));
        }

        static void move(storage_union &refSrc, storage_union &refDest) noexcept {
            // one of the conditions for using vtable_stack is a nothrow move constructor,
            // so this move constructor will never throw a exception.
            new(&refDest.stack) T(std::move(reinterpret_cast<T &>(refSrc.stack)));
            destroy(refSrc);
        }

        static void swap(storage_union &refLhs, storage_union &refRhs) noexcept {
            storage_union tmpStorage;
            move(refRhs, tmpStorage);
            move(refLhs, refRhs);
            move(tmpStorage, refLhs);
        }
    };

    /// Whether the Type T must be dynamically allocated or can be stored on the stack.
    template<typename T>
    struct requires_allocation: std::integral_constant<bool,
                                                       !(std::is_nothrow_move_constructible<T>::value// N4562 §6.3/3 [any.class]
                                                         and sizeof(T) <= sizeof(storage_union::stack) and std::alignment_of<T>::value <= std::alignment_of<storage_union::stack_storage_t>::value)> {};

    /// Returns the pointer to the vtable of the Type T.
    template<typename T>
    static vtable_type *vtable_for_type() {
        using VTableType = typename std::conditional<requires_allocation<T>::value, vtable_dynamic<T>, vtable_stack<T>>::type;
        static vtable_type table = {
#ifndef ANY_IMPL_NO_RTTI
                VTableType::type,
#endif
                VTableType::destroy,
                VTableType::copy,
                VTableType::move,
                VTableType::swap,
        };
        return &table;
    }

 protected:
    template<typename T>
    friend const T *any_cast(const any *ptrOperand) noexcept;
    template<typename T>
    friend T *any_cast(any *ptrOperand) noexcept;

#ifndef ANY_IMPL_NO_RTTI
    /// Same effect as is_same(this->Type(), t);
    bool is_typed(const std::type_info &t) const {
        return is_same(this->Type(), t);
    }
#endif

#ifndef ANY_IMPL_NO_RTTI
    /// Checks if two Type infos are the same.
    ///
    /// If ANY_IMPL_FAST_TYPE_INFO_COMPARE is defined, checks only the address of the
    /// Type infos, otherwise does an actual comparision. Checking addresses is
    /// only a valid approach when there's no interaction with outside sources
    /// (other shared libraries and such).
    static bool is_same(const std::type_info &a, const std::type_info &b) {
#ifdef ANY_IMPL_FAST_TYPE_INFO_COMPARE
        return &a == &b;
#else
        return a == b;
#endif
    }
#endif

    /// Casts (with no type_info checks) the storage pointer as const T*.
    template<typename T>
    const T *cast() const noexcept {
        return requires_allocation<typename std::decay<T>::type>::value ? reinterpret_cast<const T *>(storage.dynamic) : reinterpret_cast<const T *>(&storage.stack);
    }

    /// Casts (with no type_info checks) the storage pointer as T*.
    template<typename T>
    T *cast() noexcept {
        return requires_allocation<typename std::decay<T>::type>::value ? reinterpret_cast<T *>(storage.dynamic) : reinterpret_cast<T *>(&storage.stack);
    }

 private:
    storage_union storage;// on offset(0) so no padding for align
    vtable_type *vtable;

    template<typename T, typename... Args>
    typename std::enable_if<requires_allocation<T>::value>::type do_emplace(Args &&...args) {
        storage.dynamic = new T(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    typename std::enable_if<!requires_allocation<T>::value>::type do_emplace(Args &&...args) {
        new(&storage.stack) T(std::forward<Args>(args)...);
    }

    template<typename ValueType, typename... Args>
    void emplace_construct(Args &&...args) {
        using T = typename std::decay<ValueType>::type;

        this->vtable = vtable_for_type<T>();

        do_emplace<T>(std::forward<Args>(args)...);
    }

    template<typename ValueType, typename T>
    typename std::enable_if<requires_allocation<T>::value>::type
    do_construct(ValueType &&refValue) {
        storage.dynamic = new T(std::forward<ValueType>(refValue));
    }

    template<typename ValueType, typename T>
    typename std::enable_if<!requires_allocation<T>::value>::type
    do_construct(ValueType &&refValue) {
        new(&storage.stack) T(std::forward<ValueType>(refValue));
    }

    /// Chooses between stack and dynamic allocation for the Type decay_t<ValueType>,
    /// assigns the correct vtable, and constructs the object on our storage.
    template<typename ValueType>
    void construct(ValueType &&refValue) {
        using T = typename std::decay<ValueType>::type;

        this->vtable = vtable_for_type<T>();

        do_construct<ValueType, T>(std::forward<ValueType>(refValue));
    }
};


namespace detail {
template<typename ValueType>
inline ValueType any_cast_move_if_true(typename std::remove_reference<ValueType>::type *p, std::true_type) {
    return std::move(*p);
}

template<typename ValueType>
inline ValueType any_cast_move_if_true(typename std::remove_reference<ValueType>::type *p, std::false_type) {
    return *p;
}
}// namespace detail

/// Performs *any_cast<add_const_t<remove_reference_t<ValueType>>>(&operand), or throws bad_any_cast on failure.
template<typename ValueType>
inline ValueType any_cast(const any &refOperand) {
    auto p = any_cast<typename std::add_const<typename std::remove_reference<ValueType>::type>::type>(&refOperand);
#ifndef ANY_IMPL_NO_EXCEPTIONS
    if(p == nullptr) throw bad_any_cast();
#endif
    return *p;
}

/// Performs *any_cast<remove_reference_t<ValueType>>(&operand), or throws bad_any_cast on failure.
template<typename ValueType>
inline ValueType any_cast(any &refOperand) {
    auto p = any_cast<typename std::remove_reference<ValueType>::type>(&refOperand);
#ifndef ANY_IMPL_NO_EXCEPTIONS
    if(p == nullptr) throw bad_any_cast();
#endif
    return *p;
}

///
/// If ValueType is MoveConstructible and isn't a lvalue reference, performs
/// std::move(*any_cast<remove_reference_t<ValueType>>(&operand)), otherwise
/// *any_cast<remove_reference_t<ValueType>>(&operand). Throws bad_any_cast on failure.
///
template<typename ValueType>
inline ValueType any_cast(any &&refOperand) {
    using canMove = std::integral_constant<bool, std::is_move_constructible<ValueType>::value and !std::is_lvalue_reference<ValueType>::value>;

    auto p = any_cast<typename std::remove_reference<ValueType>::type>(&refOperand);
#ifndef ANY_IMPL_NO_EXCEPTIONS
    if(p == nullptr) throw bad_any_cast();
#endif
    return detail::any_cast_move_if_true<ValueType>(p, canMove());
}

/// If operand != nullptr and operand->Type() == typeid(ValueType), a pointer to the object
/// contained by operand, otherwise nullptr.
template<typename ValueType>
inline const ValueType *any_cast(const any *ptrOperand) noexcept {
    using T = typename std::decay<ValueType>::type;

#ifndef ANY_IMPL_NO_RTTI
    if(ptrOperand and ptrOperand->is_typed(typeid(T)))
#else
    if(operand and operand->vtable == any::vtable_for_type<T>())
#endif
        return ptrOperand->cast<ValueType>();
    else
        return nullptr;
}

/// If operand != nullptr and operand->Type() == typeid(ValueType), a pointer to the object
/// contained by operand, otherwise nullptr.
template<typename ValueType>
inline ValueType *any_cast(any *ptrOperand) noexcept {
    using T = typename std::decay<ValueType>::type;

#ifndef ANY_IMPL_NO_RTTI
    if(ptrOperand and ptrOperand->is_typed(typeid(T)))
#else
    if(operand and operand->vtable == any::vtable_for_type<T>())
#endif
        return ptrOperand->cast<ValueType>();
    else
        return nullptr;
}

inline void swap(any &refLhs, any &refRhs) noexcept {
    refLhs.swap(refRhs);
}

template<typename T, typename... Args>
any make_any(Args &&...args) {
    return any(in_place_type_t<T>{}, std::forward<Args>(args)...);
}

template<typename T, typename U, typename... Args>
any make_any(std::initializer_list<U> il, Args &&...args) {
    return any(in_place_type_t<T>{}, il, std::forward<Args>(args)...);
}
}// namespace linb

#endif// BEHAVIORTREE_ANY_HPP
