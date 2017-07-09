/** -*- C++ -*-
    @file utils/consumer.h
    @author Peter Rockai <me@mornfall.net>
*/

#include <apt-front/forward.h>
#include <apt-front/utils/multitype.h>
#include <apt-front/utils/range.h>
#include <iterator>

#ifndef APTFRONT_UTILS_CONSUMER_H
#define APTFRONT_UTILS_CONSUMER_H

namespace aptFront {
namespace utils {

template< typename T >
struct ConsumerBase : MultiTypeBase< ConsumerBase< T > >
{
    typedef T InputType;
    virtual void consume( const T &a ) = 0;
    virtual void consume( Range< T > s ) {
        while (s != s.end()) {
            consume( s.current() );
            ++s;
        }
    }
    operator Consumer< T >() const;
};

template< typename T, typename Self, typename Base = ConsumerBase< T > >
struct ConsumerImpl: MultiTypeImpl< Self, Base >
{
};

template< typename T >
struct Consumer: MultiType< Consumer< T >, ConsumerBase< T > >
{
    typedef MultiType< Consumer< T >, ConsumerBase< T > > Super;

    Consumer( const ConsumerBase< T > *i ) : Super( i ) {}
    Consumer( const Consumer &i ) : Super( i ) {}
    Consumer() {}

    typedef std::output_iterator_tag iterator_category;
    typedef void value_type;
    typedef void difference_type;
    typedef void pointer;
    typedef void reference;

    // output iterator - can't read or move
    Consumer &operator++() { return *this; }
    Consumer &operator++(int) { return *this; }
    Consumer &operator*() { return *this; }
    Consumer &operator=( const T &a ) {
        this->m_impl->consume( a );
        return *this;
    }
};

template< typename T >
inline ConsumerBase< T >::operator Consumer< T >() const
{
    return Consumer< T >( this );
}

template< typename T, typename Out >
struct ConsumerFromIterator : ConsumerImpl<
    T, ConsumerFromIterator< T, Out > >
{
    ConsumerFromIterator( Out out ) : m_out( out ) {}
    virtual void consume( const T& a ) {
        *m_out = a;
        ++m_out;
    }
protected:
    Out m_out;
};

template< typename T, typename Out >
Consumer< T > consumer( Out out ) {
    return ConsumerFromIterator< T, Out >( out );
}

template< typename Base >
Consumer< typename Base::InputType > consumer( Base b ) {
    return static_cast<Consumer< typename Base::InputType > >( b );
}

}
}

#endif
