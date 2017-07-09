/** -*- C++ -*-
    @file utils/range.h
    @author Peter Rockai <me@mornfall.net>
*/

#include <apt-front/forward.h>
#include <apt-front/utils/multitype.h>
#include <apt-front/utils/shared.h>
#include <iterator>
#include <vector>
#include <set>
#include <apt-front/error.h>
#include <algorithm>
#include <ext/algorithm>

#ifndef APTFRONT_UTILS_RANGE_H
#define APTFRONT_UTILS_RANGE_H

namespace aptFront {
namespace utils {

typedef bool SortabilityTag;

template< typename T, typename I >
struct ExtIteratorTraits {
    typedef SortabilityTag Unsortable;
};

template< typename T >
struct ExtIteratorTraits< T, typename std::set< T >::iterator > {
    typedef SortabilityTag Sorted;
};

template< typename T >
struct ExtIteratorTraits< T, typename std::vector< T >::iterator > {
    typedef SortabilityTag Sortable;
};

template< typename T >
struct RangeBase : MultiTypeBase< RangeBase< T > >
{
    typedef T value_type;
    typedef ptrdiff_t difference_type;
    typedef T *pointer;
    typedef T &reference;
    typedef const T &const_reference;

    virtual T current() const = 0;
    virtual void advance() = 0;
    virtual bool isSorted() const { return false; }
    virtual Range< T > sorted() const = 0;
    virtual Range< T > next_impl() const = 0;
    virtual Range< T > end_impl() const = 0;
    operator Range< T >() const;
    virtual bool empty() const = 0;
    virtual bool contains_impl( const T & ) const = 0;
};

template< typename T >
struct RangeProxy {
    RangeProxy( T _x ) : x( _x ) {}
    T x;
    const T *operator->() const { return &x; }
};

template< typename T, typename Self, typename Base = RangeBase< T > >
struct RangeImpl: MultiTypeImpl< Self, Base, equalityComparable >
{
    typedef T ElementType;

    typedef std::forward_iterator_tag iterator_category;
    typedef T value_type;
    typedef ptrdiff_t difference_type;
    typedef T *pointer;
    typedef T &reference;
    typedef const T &const_reference;

    RangeProxy< T > operator->() const {
        return RangeProxy< T >(this->self().current()); }
    Self next() const { Self n( this->self() ); n.advance(); return n; }
    Self begin() const { return this->self(); } // STL-style iteration
    Self end() const { Self e( this->self() ); e.gotoEnd(); return e; }
    Self &operator++() { this->self().advance(); return this->self(); }
    T operator*() const { return this->self().current(); }
    virtual Range< T > sorted() const;
    virtual Range< T > end_impl() const {
        return this->self().end();
    }
    virtual Range< T > next_impl() const {
        return this->self().next();
    }
    virtual bool empty() const {
        return this->self() == this->self().end();
    }
    virtual bool contains_impl( const T &t ) const { return this->self().contains( t ); }

    bool contains( const T &t ) const { // errrm...
        return std::find( this->self(), this->self().end(), t ) != this->self().end();
    }

    virtual void gotoEnd() = 0;
};

template< typename T >
struct Range : MultiType< Range< T >, RangeBase< T > >
{
    typedef MultiType< Range< T >, RangeBase< T > > Super;
    typedef T ElementType;

    Range( const RangeBase< T > *i ) : Super( i ) {}
    Range( const Range &i ) : Super( i ) {}
    Range() {}

    typedef std::forward_iterator_tag iterator_category;
    typedef T value_type;
    typedef ptrdiff_t difference_type;
    typedef T *pointer;
    typedef T &reference;
    typedef const T &const_reference;

    bool empty() const {
        if (this->impl())
            return this->impl()->empty();
        return true;
    }

    /* struct Proxy {
        Proxy( T _x ) : x( _x ) {}
        T x;
        const T *operator->() const { return &x; }
        }; */

    T current() const { return this->m_impl->current(); }
    Range end() const { return this->m_impl->end_impl(); }
    Range next() const { return this->m_impl->next_impl(); }
    bool contains( const T &t ) const { return this->m_impl->contains_impl( t ); }
    void advance() { this->m_impl->advance(); }
    bool isSorted() const { return this->m_impl->isSorted(); }
    Range< T > sorted() const { return this->m_impl->sorted(); }
    T operator*() const { return this->m_impl->current(); }
    RangeProxy< T > operator->() const {
        return RangeProxy< T >(this->m_impl->current()); }
    Range &operator++() { this->m_impl->advance(); return *this; }
    Range operator++(int) {
        Range< T > tmp = *this;
        this->m_impl->advance();
        return tmp;
    }
    void output( Consumer< T > t ) const {
        std::copy( *this, end(), t );
    }

    template< typename C >
    operator Range< C >();
};

template< typename T >
inline RangeBase< T >::operator Range< T >() const
{
    return Range< T >( this );
}

template< typename R >
Range< typename R::ElementType > range( R r ) {
    return r;
}

}
}

// ----- individual range implementations follow
#include <apt-front/utils/consumer.h>

namespace aptFront {
namespace utils {

// sfinae: substitution failure is not an error
template< typename T, typename I >
typename ExtIteratorTraits< T, I >::Unsortable isSortedT( I, I ) {
    return false;
}

template< typename T, typename I >
typename ExtIteratorTraits< T, I >::Sorted isSortedT( I, I ) {
    return true;
}

template< typename T, typename I >
typename ExtIteratorTraits< T, I >::Sortable isSortedT( I a, I b ) {
    if (__gnu_cxx::is_sorted( a, b ))
        return true;
    return false;
}

template< typename In >
struct IteratorRange : public RangeImpl<
    typename std::iterator_traits< In >::value_type,
    IteratorRange< In > >
{
    typedef typename std::iterator_traits< In >::value_type Value;
    typedef std::forward_iterator_tag iterator_category;
    // typedef typename std::iterator_traits< In >::iterator_category iterator_category;
    IteratorRange( In c, In e )
        : m_current( c ), m_end( e ) {}
    virtual Value current() const {
        return *m_current;
    }
    virtual void advance() {
        ++m_current;
    }
    bool operator==( const IteratorRange &r ) const {
        return r.m_current == m_current && r.m_end == m_end;
    }
    void gotoEnd() {
        m_current = m_end;
    }

    bool isSorted() const {
        return isSortedT< Value, In >( m_current, m_end );
    }

protected:
    In m_current, m_end;
};

template< typename T, typename Casted >
struct CastedRange : public RangeImpl< T, CastedRange< T, Casted > >
{
    CastedRange( Range< Casted > r ) : m_casted( r ) {}
    virtual T current() const {
        return static_cast< T >( m_casted.current() );
    }
    virtual void advance() { m_casted.advance(); }

    bool operator==( const CastedRange &r ) const {
        return m_casted == r.m_casted; }

    void gotoEnd() {
        m_casted = m_casted.end();
    }

    bool isSorted() const { // XXX actually, this is probably wrong
        return m_casted.isSorted();
    }
protected:
    Range< Casted > m_casted;
};

template< typename T, typename C >
Range< T > castedRange( C r ) {
    return CastedRange< T, typename C::ElementType >( r );
}

// compat
template< typename T, typename C >
Range< T > upcastRange( C r ) {
    return CastedRange< T, typename C::ElementType >( r );
}

template< typename T> template< typename C >
Range< T >::operator Range< C >() {
    return castedRange< C >( *this );
}

template< typename In >
Range< typename In::value_type > range( In b, In e ) {
    return IteratorRange< In >( b, e );
}


template< typename T>
struct SharedVector : std::vector< T >, SharedBase
{
};

template< typename T >
struct IntersectionRange : RangeImpl< T, IntersectionRange< T > >
{
    IntersectionRange() {}
    IntersectionRange( Range< T > r1, Range< T > r2 )
        : m_first( r1.sorted() ), m_second( r2.sorted() ),
        m_valid( false )
    {
    }

    void find() const {
        if (!m_valid) {
            while ( m_first != m_first.end()
                    && m_second != m_second.end() ) {
                if ( *m_first < *m_second )
                    ++m_first;
                else if ( *m_second < *m_first )
                    ++m_second;
                else break; // equal
            }
            if ( m_second == m_second.end() ) m_first = m_first.end();
            else if ( m_first == m_first.end() ) m_second = m_second.end();
        }
        m_valid = true;
    }

    virtual void advance() {
        find();
        ++m_first;
        ++m_second;
        m_valid = false;
    }

    virtual T current() const {
        find();
        return m_first.current();
    }

    void gotoEnd() {
        m_first = m_first.end();
        m_second = m_second.end();
    }

    bool operator==( const IntersectionRange &f ) const {
        find();
        f.find();
        return m_first == f.m_first;
        // return m_pred == f.m_pred && m_range == f.m_range;
    }

    bool isSorted() const { return true; }

protected:
    mutable Range< T > m_first, m_second;
    mutable bool m_valid:1;
};

template< typename R >
IntersectionRange< typename R::ElementType > intersectionRange( R r1, R r2 ) {
    return IntersectionRange< typename R::ElementType >( r1, r2 );
}

template< typename R, typename Pred >
struct FilteredRange : RangeImpl< typename R::ElementType,
                                  FilteredRange< R, Pred > >
{
    typedef typename R::ElementType ElementType;
    FilteredRange( const R &r, Pred p ) : m_range( r ), m_pred( p ),
        m_valid( false ) {}

    void find() const {
        if (!m_valid)
            m_range = std::find_if( m_range, m_range.end(), m_pred );
        m_valid = true;
    }

    virtual void advance() {
        find();
        ++m_range;
        m_valid = false;
    }

    virtual ElementType current() const {
        find();
        return m_range.current();
    }

    void gotoEnd() {
        m_range = m_range.end();
    }

    bool operator==( const FilteredRange &f ) const {
        find();
        f.find();
        return m_range == f.m_range;
        // return m_pred == f.m_pred && m_range == f.m_range;
    }

    bool isSorted() const {
        return m_range.isSorted();
    }

protected:
    mutable R m_range;
    Pred m_pred;
    mutable bool m_valid:1;
};

template< typename R, typename Pred >
FilteredRange< R, Pred > filteredRange(
    R r, Pred p ) {
    return FilteredRange< R, Pred >( r, p );
}

template< typename T >
struct UniqueRange : RangeImpl< T, UniqueRange< T > >
{
    UniqueRange() {}
    UniqueRange( Range< T > r ) : m_range( r.sorted() ), m_valid( false ) {}

    void find() const {
        if (!m_valid)
            while ( m_range != m_range.end()
                    && m_range.next() != m_range.end()
                    && *m_range == *m_range.next() )
                m_range = m_range.next();
        m_valid = true;
    }

    virtual void advance() {
        find();
        ++m_range;
        m_valid = false;
    }

    virtual T current() const {
        find();
        return m_range.current();
    }

    void gotoEnd() {
        m_range = m_range.end();
    }

    bool operator==( const UniqueRange &r ) const {
        find();
        r.find();
        return m_range == r.m_range;
    }

    bool isSorted() const {
        return true;
    }

protected:
    mutable Range< T > m_range;
    mutable bool m_valid:1;
};

template< typename R >
UniqueRange< typename R::ElementType > uniqueRange( R r1 ) {
    return UniqueRange< typename R::ElementType >( r1 );
}

template< typename Transform >
struct TransformedRange : RangeImpl< typename Transform::result_type,
                                     TransformedRange< Transform > >
{
    typedef typename Transform::argument_type Source;
    typedef typename Transform::result_type Result;
    TransformedRange( Range< Source > r, Transform t )
        : m_range( r ), m_transform( t ) {}

    bool operator==( const TransformedRange &o ) const {
        return m_range == o.m_range; // XXX buaaaahaaa!
    }

    Result current() const {
        return m_transform( m_range.current() );
    }

    void advance() {
        ++m_range;
    }

    void gotoEnd() {
        m_range = m_range.end();
    }

protected:
    Range< Source > m_range;
    Transform m_transform;
};

template< typename Trans >
TransformedRange< Trans > transformedRange(
    Range< typename Trans::argument_type > r, Trans t ) {
    return TransformedRange< Trans >( r, t );
}

template< typename Container >
struct BackedRange :
    RangeImpl< typename Container::value_type, BackedRange< Container > >
// ConsumerImpl< typename Cont::value_type, BackedRange< Cont > >
{
    typedef typename Container::value_type T;
    typedef BackedRange< Container > This;
    struct SharedContainer : Container, SharedBase {
        template< typename I >
        SharedContainer( I a, I b ) : Container( a, b ) {}
    };
    virtual void advance() { ++m_position; }

    virtual T current() const {
        return *m_position;
    }
    virtual void gotoEnd() {
        m_position = m_container->end();
    }

protected:
    typedef SharedPtr< SharedContainer > ContainerPtr;
    typedef typename Container::const_iterator Position;
    Position m_position;
    ContainerPtr m_container;
};

// XXX using VectorRange as an output iterator compiles but DOES NOT
// WORK (segfaults)... you can use consumer( myPetVectorRange ) in the
// meantime
// this needs fixing though
template< typename T >
struct VectorRange : RangeImpl< T, VectorRange< T > >,
    ConsumerImpl< T, VectorRange< T > >
{
    typedef std::random_access_iterator_tag iterator_category;
    VectorRange() : m_vector( new Vector ), m_position( 0 ) {}
    VectorRange( const Range< T > &i ) {
        RangeImpl< T, VectorRange< T > >::initFromBase( i.impl() );
    }

    virtual void consume( const T &a ) {
        m_vector->push_back( a );
    }

    virtual void advance() {
        ++m_position;
    }

    virtual T current() const {
        return m_vector->operator[]( m_position );
    }

    void gotoEnd() {
        m_position = std::distance( m_vector->begin(), m_vector->end() );
    }

    bool operator==( const VectorRange &r ) const {
        return m_position == r.m_position;
    }

    VectorRange &operator+=( ptrdiff_t off ) {
        m_position += off;
        return *this;
    }
    VectorRange &operator--() {
        --m_position;
        return *this;
    }
    VectorRange operator--( int ) {
        VectorRange tmp( *this );
        --m_position;
        return tmp;
    }

    ptrdiff_t operator-( const VectorRange &r ) {
        return m_position - r.m_position;
    }
    VectorRange operator-( ptrdiff_t off ) {
        VectorRange< T > r( *this );
        r.m_position = m_position - off;
        return r;
    }

    VectorRange operator+( ptrdiff_t off ) {
        VectorRange< T > r( *this );
        r.m_position = m_position + off;
        return r;
    }

    bool operator<( const VectorRange &r ) {
        return m_position < r.m_position;
    }

    size_t size() const {
        return m_vector->size() - m_position;
    }

    T &operator*() {
        return m_vector->operator[]( m_position );
    }

    const T &operator*() const {
        return m_vector->operator[]( m_position );
    }

    void clear() {
        m_vector->clear();
        m_position = 0;
    }

    bool isSorted() const {
        return false; // XXX test for sortedness?
    }
protected:
    typedef SharedVector< T > Vector;
    typedef SharedPtr< Vector > VectorPointer;
    VectorPointer m_vector;
    ptrdiff_t m_position;
};

template< typename R >
Range< typename R::ElementType > sortedRange( R in ) {
    VectorRange< typename R::ElementType > out;
    std::copy( in.begin(), in.end(), consumer( out ) );
    std::sort( out.begin(), out.end() );
    return out;
}

template< typename T, typename Self, typename Base >
Range< T > RangeImpl< T, Self, Base >::sorted() const
{
    if ( this->isSorted() )
        return this->self();
    else
        return sortedRange( this->self() );
}

template< typename T, typename _Advance, typename _End >
struct GeneratedRange : RangeImpl< T, GeneratedRange< T, _Advance, _End > >
{
    typedef _Advance Advance;
    typedef _End End;

    GeneratedRange() {}
    GeneratedRange( const T &t, const Advance &a, const End &e )
        : m_current( t ), m_advance( a ), m_endPred( e ), m_end( false )
    {
    }

    void advance() {
        m_advance( m_current );
    }

    void gotoEnd() {
        m_end = true;
    }

    T current() const { return m_current; }

    bool isEnd() const { return m_end || m_endPred( m_current ); }

    bool operator==( const GeneratedRange &r ) const {
        if ( isEnd() && r.isEnd() ) return true;
        if ( isEnd() || r.isEnd() ) return false;
        return m_current == r.m_current;
    }

protected:
    T m_current;
    Advance m_advance;
    End m_endPred;
    bool m_end;
};

template< typename T, typename A, typename E >
GeneratedRange< T, A, E > generatedRange( T t, A a, E e )
{
    return GeneratedRange< T, A, E >( t, a, e );
}

}
}

#endif
