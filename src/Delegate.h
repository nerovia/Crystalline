#pragma once

template<typename T, typename ...TArgs>
class Delegate;

template<typename T, typename ...TArgs>
using Function = T(*)(TArgs...);

template<typename C, typename T, typename ...TArgs>
using MemberFunction = T(C::*)(TArgs...);

namespace Invokers
{
    template<typename T, typename ...TArgs>
    class StaticInvoker : public Delegate<T, TArgs...>
    {
    protected:
        Function<T, TArgs...> function;
    public:
        StaticInvoker(Function<T, TArgs...> function) : function(function) { }
        T invoke(TArgs... args) override { return function(args...); }
    };

    template<typename C, typename T, typename ...TArgs>
    class MemberInvoker : public Delegate<T, TArgs...>
    {
    protected:
        C* instance;
        T (C::*function)(TArgs...);
    public:
        MemberInvoker(C& instance, MemberFunction<C, T, TArgs...> function) : instance(&instance), function(function) { }
        T invoke(TArgs... args) override { return (instance->*function)(args...); }
    };

    template<typename O, typename T, typename ...TArgs>
    class ObjectInvoker : public Delegate<T, TArgs...>
    {
    protected:
        O lambda;
    public:
        ObjectInvoker(const O& lambda) : lambda(lambda) { }
        T invoke(TArgs... args) override { return (T)lambda(args...); }
    };
}

template<typename T, typename ...TArgs>
class Delegate
{
public:
    static Delegate<T, TArgs...>* create(Function<T, TArgs...> function)
    {
        return new Invokers::StaticInvoker<T, TArgs...>(function);
    }

    template<typename C>
    static Delegate<T, TArgs...>* create(C& instance, MemberFunction<C, T, TArgs...> function)
    {
        return new Invokers::MemberInvoker<C, T, TArgs...>(instance, function);
    }

    template<typename O>
    static Delegate<T, TArgs...>* create(const O& lambda)
    {
        return new Invokers::ObjectInvoker<O, T, TArgs...>(lambda);
    }

    virtual ~Delegate() { };
    virtual T invoke(TArgs...) = 0;
    T operator()(TArgs... args) { return invoke(args...); }
};

template<class T, class... TArgs>
static Delegate<T, TArgs...>* delegateOf(Function<T, TArgs...> function)
{
    return Delegate<T, TArgs...>::create(function);
}

template<class C, class T, class... TArgs>
static Delegate<T, TArgs...>* delegateOf(C& instance, MemberFunction<C, T, TArgs...> function)
{
    return Delegate<T, TArgs...>::create(instance, function);
}

template<class O, class T, class... TArgs>
static Delegate<T, TArgs...>* delegateOf(const O& lambda)
{
    return Delegate<T, TArgs...>::create(lambda);
}

template<typename T>
using Getter = Delegate<T>;

template<typename T>
using Setter = Delegate<void, T>;

using Action = Delegate<void>;

template<typename T>
class Property;

namespace Invokers
{
    template<typename T>
    class ReadonlyStaticPropertyInvoker : public Property<T>, public StaticInvoker<T>
    {
    public:
        ReadonlyStaticPropertyInvoker(Function<T> getter) : StaticInvoker<T>(getter) { }
        T get() override { return StaticInvoker<T>::invoke(); }
    };

    template<typename C, typename T>
    class ReadonlyMemberPropertyInvoker : public Property<T>, public MemberInvoker<C, T>
    {
    public:
        ReadonlyMemberPropertyInvoker(C& instance, MemberFunction<C, T> getter) : MemberInvoker<C, T>(instance, getter) { }
        T get() override { return MemberInvoker<C, T>::invoke(); }
    };

    template<typename O, typename T>
    class ReadonlyObjectPropertyInvoker : public Property<T>, public ObjectInvoker<O, T>
    {
    public:
        ReadonlyObjectPropertyInvoker(const O& getter) : ObjectInvoker<O, T>(getter) { }
        T get() override { return ObjectInvoker<O, T>::invoke(); }
    };

    template<typename T>
    class StaticPropertyInvoker : public ReadonlyStaticPropertyInvoker<T>, protected StaticInvoker<void, T>
    {
    public:
        StaticPropertyInvoker(Function<T> getter, Function<void, T> setter) : ReadonlyStaticPropertyInvoker<T>(getter), StaticInvoker<void, T>(setter) { }
        void set(T value) override { StaticInvoker<void, T>::invoke(value); }
        bool isReadonly() override { return false; }
    };

    template<typename C, typename T>
    class MemberPropertyInvoker : public ReadonlyMemberPropertyInvoker<C, T>
    {
    protected:
        void(C::* setter)(T);
    public:
        MemberPropertyInvoker(C& instance, MemberFunction<C, T> getter, MemberFunction<C, void, T> setter) : ReadonlyMemberPropertyInvoker<C, T>(instance, getter), setter(setter) { }
        void set(T value) override { (ReadonlyMemberPropertyInvoker<C, T>::instance->*setter)(value); }
        bool isReadonly() override { return false; }
    };

    template<typename O, typename T>
    class ObjectPropertyInvoker : ReadonlyObjectPropertyInvoker<O, T>, ObjectInvoker<O, void, T>
    {
    public:
        ObjectPropertyInvoker(const O& getter, const O& setter) : ReadonlyObjectPropertyInvoker<O, T>(getter), ObjectInvoker<O, void, T>(setter) { }
        void set(T value) override { ObjectInvoker<O, void, T>::invoke(value); }
        bool isReadonly() override { return false; }
    };

    template<typename T>
    class CompositPropertyInvoker : Property<T>
    {
    private:
        Getter<T> getter;
        Setter<T> setter;
    public:
        CompositPropertyInvoker(Getter<T> getter, Setter<T> setter) : getter(getter), setter(setter) { }
        T get() override { return getter.invoke(); }
        void set(T value) override { setter.invoke(value); }
        bool isReadonly() override { return false; }
    };
}

template<typename T>
class Property
{
public:
    static Property<T>* create(Function<T> getter) { return new Invokers::ReadonlyStaticPropertyInvoker<T>(getter); }

    template<typename C>
    static Property<T>* create(C& instance, MemberFunction<C, T> getter) { return new Invokers::ReadonlyMemberPropertyInvoker<C, T>(instance, getter); }

    template<typename O>
    static Property<T>* create(const O& getter) { return new Invokers::ReadonlyObjectPropertyInvoker<O, T>(getter); }

    static Property<T>* create(Function<T> getter, Function<void, T> setter) { return new Invokers::StaticPropertyInvoker<T>(getter, setter); }

    template<typename C>
    static Property<T>* create(C& instance, MemberFunction<C, T> getter, MemberFunction<C, void, T> setter)  { return new Invokers::MemberPropertyInvoker<C, T>(instance, getter, setter); }

    template<typename O>
    static Property<T>* create(const O& getter, const O& setter) { return new Invokers::ObjectPropertyInvoker<O, T>(getter, setter); }

    // static Property<T>* create(Getter<T> getter, Setter<T> setter) { return new Invokers::CompositPropertyInvoker<T>(getter, setter); }

    virtual ~Property() {};
    virtual T get() = 0;
    virtual void set(T value) { }
    virtual bool isReadonly() { return true; }
};

template<class T>
static Property<T>* propertyOf(Function<T> getter) { return Property<T>::create(getter); }

template<class T, class C>
static Property<T>* propertyOf(C& instance, MemberFunction<C, T> getter) { return Property<T>::create(instance, getter); }

template<class T, class O>
static Property<T>* propertyOf(const O& getter) { return Property<T>::create(getter); }

template<class T>
static Property<T>* propertyOf(Function<T> getter, Function<void, T> setter) { return Property<T>::create(getter, setter); }

template<class T, class C>
static Property<T>* propertyOf(C& instance, MemberFunction<C, T> getter, MemberFunction<C, void, T> setter) { return Property<T>::create(instance, getter, setter); }

template<class T, class O>
static Property<T>* propertyOf(const O& getter, const O& setter) { return Property<T>::create(getter, setter); }

// template<class T>
// static Property<T>* propertyOf(Getter<T> getter, Setter<T> setter) { return Property<T>::create(getter, setter); }
