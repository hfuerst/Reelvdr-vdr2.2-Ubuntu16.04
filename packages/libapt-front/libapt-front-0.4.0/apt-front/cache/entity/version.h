/** -*- C++ -*-
 * @file cache/entity/version.h
 * @author Peter Rockai <me@mornfall.net>
 */

#include <apt-front/cache/cache.h>
#include <apt-front/cache/entity/package.h>
#include <apt-front/cache/component/records.h>
#include <string>
#include <sstream>
#include <cmath>
#include <stdexcept>

#ifndef APTFRONT_CACHE_ENTITY_VERSION_H
#define APTFRONT_CACHE_ENTITY_VERSION_H

namespace aptFront {
namespace cache {
namespace entity {

template< typename Data >
struct VersionT : public Implementation<Version>, public Named {
public:
    VersionT() : m_data( 0 ) {}
    VersionT( const Entity &i ) {
        initFromBase( i.impl() );
    }

    inline component::Records &recordsCache() const {
        return cache().records();
    }

    /* VersionIterator functionality. */
    bool operator ==( const VersionT &v ) const {
        return ( !v.valid() && !valid() )
            || ( v.m_data == m_data && ( &v.cache() ) == ( &cache() ) );
    }

    bool valid() const;

    // FIXME
    pkgCache::VerFileIterator fileList() const;

    VersionT nextInCache() const;

    typedef DefaultArgument< std::string > DefaultString;

    exception::InternalError getterError( std::string f ) const {
        return exception::InternalError(
            std::string( "Error calling entity::Version::" ) + f
            + ", the data is not available" );
    }

    virtual Entity stable() const {
        return StableVersion( *this );
    }

    Relation firstDependInCache() const;

    std::string shortDescription( DefaultString d = DefaultString() ) const {
        if (!valid())
            return d.get( getterError( "shortDescription()" ) );
        return recordsCache().record(*this).shortDescription;
    }

    std::string longDescription( DefaultString d = DefaultString() ) const {
        if (!valid())
            return d.get( getterError( "longDescription()" ) );
        return recordsCache().record( *this ).longDescription;
    }

    std::string name( DefaultString d = DefaultString() ) const {
        if (!valid())
            return d.get( getterError( "name()" ) );
        return package().name();
    }

    std::string maintainer( DefaultString d = DefaultString() ) const {
        if (!valid())
            return d.get( getterError( "maintainer()" ) );
        return recordsCache().record( *this ).maintainer;
    }

    std::string architecture( DefaultString d = DefaultString() ) const {
        if (!valid())
            return d.get( getterError( "architecture()" ) );
        return packageCache().strPtr( m_data->Arch );
    }

    std::string section( DefaultString d = DefaultString() ) const {
        if (!valid())
            return d.get( getterError( "section()" ) );
        return packageCache().strPtr( m_data->Section );
    }

    std::string source( DefaultString d = DefaultString() ) const {
        if ( !valid() )
            return d.get( getterError( "source()" ) );
        return recordsCache().record( *this ).source;
    }

    std::string fileName( DefaultString d = DefaultString() ) const {
        if ( !valid() )
            return d.get( getterError( "fileName()" ) );
        return recordsCache().record( *this ).fileName;
    }

    std::string md5sum( DefaultString d = DefaultString() ) const {
        if ( !valid() )
            return d.get( getterError( "md5sum()" ) );
        return recordsCache().record( *this ).md5sum;
    }

    int installedSize( DefaultArgument< int > d = DefaultArgument< int >() ) const {
        if (!valid())
            return d.get( getterError( "installedSize()" ) );
        return m_data->InstalledSize;
    }
    
    std::string installedSizeString( DefaultString d = DefaultString() ) const;

    Package package() const throw();

    std::string versionString( DefaultString d = DefaultString() ) const {
        if (!valid())
            return d.get( getterError( "versionString()" ) );
        return (packageCache().strPtr( m_data->VerStr ));
    }

    /**
     * Get the complete package record for this package
     */
    std::string completeRecord();

    utils::Range< Relation > depends() const;

    // Version( pkgCache::VerIterator );
    VersionT( Cache *c, const Data &v )
        : m_data (v)
    {
        setCache( c );
    }

    // operator pkgCache::VerIterator();
    bool operator<( const VersionT &i ) const {
        return m_data < i.m_data;
    } // XXX unit test!

    Data data() const { return m_data; }
protected:
    Data m_data;
};

// typedef VersionT< pkgCache::Version * > Version;

class StableVersion :
        public Implementation<StableVersion, Version>,
        public Observer
{
public:
    StableVersion( const Version &p )
        {
            Version::initFromBase( &p );
            setCache( Version::m_cache );
            observeComponent<component::Packages>();
        }
    StableVersion( Cache *c = 0 ) : Observer (c) {
        setCache( c );
        observeComponent<component::Packages>();
    }
    StableVersion( const Entity &i ) {
        initFromBase( dynamic_cast<Version *>( i.impl() ) );
        observeComponent<component::Packages>();
    }
    virtual ~StableVersion() {}
    void notifyPreRebuild( component::Base *c );
    void notifyPostRebuild( component::Base *c );
    virtual void setCache( Cache *c ) {
        Version::setCache( c );
        Observer::setCache( c );
    }
protected:
    std::string m_pkgName;
    std::string m_pkgVersion;
};

}
}
}

#include <apt-front/cache/component/state.h>
#include <apt-front/cache/entity/relation.h>

namespace aptFront {
namespace cache {
namespace entity {

template< typename T >
Package VersionT< T >::package() const throw() {
    if (!valid())
        return Package();
    return packageCache().packageForVersion( *this );
}

template< typename T >
std::string VersionT< T >::installedSizeString( DefaultString d ) const {
        if (!valid())
            return d.get( getterError( "installedSizeString()" ) );
        return component::State::sizeString( installedSize() );
    }

template< typename T >
bool VersionT< T >::valid() const
{
    return (m_data)
        && (m_data!= packageCache().verPtr());
}

template< typename T >
pkgCache::VerFileIterator VersionT< T >::fileList() const
{
    return pkgCache::VerFileIterator
        ( packageCache(),
          packageCache().verFilePtr( m_data->FileList ) );
} // {return VerFileIterator(*Owner,Owner->VerFileP + Ver->FileList);};

template< typename T >
VersionT< T > VersionT< T >::nextInCache() const
{
    Version v = *this;
    v.m_data = packageCache().verPtr(
        m_data->NextVer );
    return v;
} // {if (Ver != Owner->VerP) Ver = Owner->VerP + Ver->NextVer;};

template< typename T >
inline utils::Range< Relation > VersionT< T >::depends() const {
    utils::VectorRange< Relation > r;
    Relation rel = firstDependInCache();
    while (rel.valid()) {
        r.consume( rel );
        rel = rel.nextInCache();
    }
    return r;
}

template< typename T >
inline Relation VersionT< T >::firstDependInCache() const {
    checkValid(); // errr...
    return Relation( m_cache,
                     packageCache().depPtr(
                         m_data->DependsList ) );
}

}
}
}
// vim:set ts=4 sw=4:
#endif
