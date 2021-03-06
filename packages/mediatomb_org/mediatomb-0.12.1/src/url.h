/*MT*
    
    MediaTomb - http://www.mediatomb.cc/
    
    url.h - this file is part of MediaTomb.
    
    Copyright (C) 2005 Gena Batyan <bgeradz@mediatomb.cc>,
                       Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>
    
    Copyright (C) 2006-2010 Gena Batyan <bgeradz@mediatomb.cc>,
                            Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>,
                            Leonhard Wimmer <leo@mediatomb.cc>
    
    MediaTomb is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.
    
    MediaTomb is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    version 2 along with MediaTomb; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
    
    $Id: url.h 2081 2010-03-23 20:18:00Z lww $
*/

/// \file url.h
/// \brief Definition of the URLL class.

#ifdef HAVE_CURL

#ifndef __URL_H__
#define __URL_H__

#include <curl/curl.h>
#include "zmmf/zmmf.h"
#include "zmm/zmm.h"

class URL : public zmm::Object
{
public:
    /// \brief This is a simplified version of the File_Info class as used
    /// in libupnp.
    class Stat : public zmm::Object
    {
    public:
        /// \brief Iinitalizes the class with given values, the values
        /// can not be changed afterwards.
        ///
        /// \param size size of the media in bytes
        /// \param mimetype mime type of the media
        Stat(zmm::String url, off_t size, zmm::String mimetype)
        {
            this->url = url; this->size = size; this->mimetype = mimetype; 
        }

        zmm::String getURL()        { return url; }
        off_t getSize()             { return size; }
        zmm::String getMimeType()   { return mimetype; }
    
    protected:
        zmm::String url;
        off_t size;
        zmm::String mimetype;
    };
    /// \brief Constructor allowing to hint the buffer size.
    /// \param buffer_hint size of the buffer that will be preallocated
    /// for the download, if too small it will be realloced.
    URL(size_t buffer_hint = 1024*1024);

    /// \brief downloads either the content or the headers to the buffer.
    ///
    /// This function uses an already initialized curl_handle, the reason
    /// is, that curl might keep the connection open if we do subsequent
    /// requests to the same server.
    /// 
    /// \param curl_handle an initialized and ready to use curl handle
    /// \param URL
    /// \param only_header set true if you only want the header and not the
    /// body
    /// \param vebose enable curl verbose option
    zmm::Ref<zmm::StringBuffer> download(zmm::String URL,
                                         long *HTTP_retcode, 
                                         CURL *curl_handle = NULL,
                                         bool only_header=false,
                                         bool verbose=false,
                                         bool redirect=false);

    zmm::Ref<Stat> getInfo(zmm::String URL, CURL *curl_handle = NULL );
protected:
    size_t buffer_hint;
    pthread_t pid;

    /// \brief This function is installed as a callback for libcurl, when
    /// we download data from a remote site.
    static size_t dl(void *buf, size_t size, size_t nmemb, void *data);
};

#endif//__URL_H__

#endif//HAVE_CURL
