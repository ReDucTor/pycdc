#ifndef _PYC_OBJECT_H
#define _PYC_OBJECT_H

#include <cstddef>
#include <type_traits>

template <class ObjT>
class PycRef {
    template<class AnyObjT>
    friend class PycRef;
public:
    PycRef() : m_obj(nullptr) { }
    PycRef(std::nullptr_t) : m_obj(nullptr) { }

    template<class AnyObjT>
    PycRef(AnyObjT* obj) : m_obj(obj)
    {
        if (m_obj)
            m_obj->addRef();
    }

    template<class AnyObjT>
    PycRef(const PycRef<AnyObjT>& obj) : m_obj(obj.m_obj)
    {
        if (m_obj)
            m_obj->addRef();
    }

    PycRef(const PycRef& obj) : m_obj(obj.m_obj)
    {
        if (m_obj)
            m_obj->addRef();
    }

    ~PycRef()
    {
        if (m_obj)
            m_obj->delRef();
    }

    template<class AnyObjT>
    PycRef& operator=(AnyObjT* obj)
    {
        if (obj)
            obj->addRef();
        if (m_obj)
            m_obj->delRef();
        m_obj = obj;
        return *this;
    }

    template<class AnyObjT>
    PycRef& operator=(const PycRef<AnyObjT> & obj)
    {
        if (obj.m_obj)
            obj.m_obj->addRef();
        if (m_obj)
            m_obj->delRef();
        m_obj = obj.m_obj;
        return *this;
    }

    PycRef& operator=(const PycRef& obj)
    {
        if (obj.m_obj)
            obj.m_obj->addRef();
        if (m_obj)
            m_obj->delRef();
        m_obj = obj.m_obj;
        return *this;
    }

    ObjT& operator*() const { return *m_obj; }
    ObjT* operator->() const { return m_obj; }
    operator ObjT*() const { return m_obj; }
    ObjT* get() const { return m_obj; }

    /* This is just for coding convenience -- no type checking is done! */
    template <class CastT, typename = typename std::enable_if<
        std::is_base_of<ObjT, CastT>::value >::type>
    PycRef<CastT> cast() const { return static_cast<CastT*>(m_obj); }

private:
    ObjT* m_obj;
};

template<class T, class U>
inline bool operator==(const PycRef<T>& lhs, const PycRef<U>& rhs)
{
    return lhs.get() == rhs.get();
}

template<class T, class U>
inline bool operator!=(const PycRef<T>& lhs, const PycRef<U>& rhs)
{
    return !(lhs == rhs);
}


class PycData;
class PycModule;

/* Please only hold PycObjects inside PycRefs! */
class PycObject {
public:
    enum Type {
        // From the Python Marshallers
        TYPE_NULL = '0',
        TYPE_NONE = 'N',
        TYPE_FALSE = 'F',
        TYPE_TRUE = 'T',
        TYPE_STOPITER = 'S',
        TYPE_ELLIPSIS = '.',
        TYPE_INT = 'i',
        TYPE_INT64 = 'I',
        TYPE_FLOAT = 'f',
        TYPE_BINARY_FLOAT = 'g',
        TYPE_COMPLEX = 'x',
        TYPE_BINARY_COMPLEX = 'y',
        TYPE_LONG = 'l',
        TYPE_STRING = 's',
        TYPE_INTERNED = 't',
        TYPE_STRINGREF = 'R',
        TYPE_OBREF = 'r',
        TYPE_TUPLE = '(',
        TYPE_LIST = '[',
        TYPE_DICT = '{',
        TYPE_CODE = 'c',
        TYPE_CODE2 = 'C',   // Used in Python 1.0 - 1.2
        TYPE_UNICODE = 'u',
        TYPE_UNKNOWN = '?',
        TYPE_SET = '<',
        TYPE_FROZENSET = '>',
        TYPE_ASCII = 'a',
        TYPE_ASCII_INTERNED = 'A',
        TYPE_SMALL_TUPLE = ')',
        TYPE_SHORT_ASCII = 'z',
        TYPE_SHORT_ASCII_INTERNED = 'Z',
    };

    PycObject(int type = TYPE_UNKNOWN) : m_refs(0), m_type(type) { }
    virtual ~PycObject() { }

    int type() const { return internalGetType(this); }

    virtual bool isEqual(PycRef<PycObject> obj) const
    { return (this == (PycObject*)obj); }

    virtual void load(PycData*, PycModule*) { }

private:
    int m_refs;
    int m_type;

    // Hack to make clang happy :(
    static int internalGetType(const PycObject *obj)
    {
        return obj ? obj->m_type : TYPE_NULL;
    }

public:
    void addRef() { ++m_refs; }
    void delRef() { if (--m_refs == 0) delete this; }
};

PycRef<PycObject> CreateObject(int type);
PycRef<PycObject> LoadObject(PycData* stream, PycModule* mod);

/* Static Singleton objects */
extern PycRef<PycObject> Pyc_NULL;
extern PycRef<PycObject> Pyc_None;
extern PycRef<PycObject> Pyc_Ellipsis;
extern PycRef<PycObject> Pyc_StopIteration;
extern PycRef<PycObject> Pyc_False;
extern PycRef<PycObject> Pyc_True;

#endif
