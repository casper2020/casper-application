// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/browser/image_cache.h"

#include "cef3/shared/browser/utils/file_util.h"

#include "cef3/shared/browser/utils/resource_util.h"

static const char kEmptyId[] = "__empty";

#ifdef __APPLE__
#pragma mark ImageCache
#endif

casper::cef3::client::browser::ImageCache::ImageCache ()
{
    
}

casper::cef3::client::browser::ImageCache::~ImageCache ()
{
    CEF_REQUIRE_UI_THREAD();
}

#ifdef __APPLE__
#pragma mark ImageCache::ImageRep
#endif

casper::cef3::client::browser::ImageCache::ImageRep::ImageRep (const std::string& path, float scale_factor)
: path_(path), scale_factor_(scale_factor)
{
    DCHECK(!path_.empty());
    DCHECK_GT(scale_factor_, 0.0f);
}

#ifdef __APPLE__
#pragma mark ImageCache::ImageInfo
#endif

casper::cef3::client::browser::ImageCache::ImageInfo::ImageInfo (const std::string& id, const ImageRepSet& reps,
                                                                 bool internal, bool force_reload)
: id_(id), reps_(reps), internal_(internal), force_reload_(force_reload)
{
#ifndef NDEBUG
    DCHECK(!id_.empty());
    if (id_ != kEmptyId)
        DCHECK(!reps_.empty());
#endif
}

// static
casper::cef3::client::browser::ImageCache::ImageInfo casper::cef3::client::browser::ImageCache::ImageInfo::Empty ()
{
    return ImageInfo(kEmptyId, ImageRepSet(), true, false);
}

// static
casper::cef3::client::browser::ImageCache::ImageInfo casper::cef3::client::browser::ImageCache::ImageInfo::Create1x (const std::string& id,
                                                                                                                     const std::string& path_1x,
                                                                                                                     bool internal)
{
    ImageRepSet reps;
    reps.push_back(ImageRep(path_1x, 1.0f));
    return ImageInfo(id, reps, internal, false);
}

// static
casper::cef3::client::browser::ImageCache::ImageInfo casper::cef3::client::browser::ImageCache::ImageInfo::Create2x (const std::string& id,
                                                                                                                     const std::string& path_1x, const std::string& path_2x,
                                                                                                                     bool internal)
{
    ImageRepSet reps;
    reps.push_back(ImageRep(path_1x, 1.0f));
    reps.push_back(ImageRep(path_2x, 2.0f));
    return ImageInfo(id, reps, internal, false);
}

// static
casper::cef3::client::browser::ImageCache::ImageInfo casper::cef3::client::browser::ImageCache::ImageInfo::Create2x (const std::string& id)
{
    return Create2x(id, id + ".1x.png", id + ".2x.png", true);
}

void casper::cef3::client::browser::ImageCache::LoadImages (const ImageInfoSet& image_info, const LoadImagesCallback& callback)
{
    DCHECK(!image_info.empty());
    DCHECK(!callback.is_null());
    
    if (!CefCurrentlyOn(TID_UI)) {
        CefPostTask(TID_UI, base::Bind(&ImageCache::LoadImages, this, image_info,
                                       callback));
        return;
    }
    
    ImageSet images;
    bool missing_images = false;
    
    ImageInfoSet::const_iterator it = image_info.begin();
    for (; it != image_info.end(); ++it) {
        const ImageInfo& info = *it;
        
        if (info.id_ == kEmptyId) {
            // Image intentionally left empty.
            images.push_back(NULL);
            continue;
        }
        
        ImageMap::iterator it2 = image_map_.find(info.id_);
        if (it2 != image_map_.end()) {
            if (!info.force_reload_) {
                // Image already exists.
                images.push_back(it2->second);
                continue;
            }
            
            // Remove the existing image from the map.
            image_map_.erase(it2);
        }
        
        // Load the image.
        images.push_back(NULL);
        if (!missing_images)
            missing_images = true;
    }
    
    if (missing_images) {
        CefPostTask(TID_FILE, base::Bind(&ImageCache::LoadMissing, this, image_info,
                                         images, callback));
    } else {
        callback.Run(images);
    }
}

CefRefPtr<CefImage> casper::cef3::client::browser::ImageCache::GetCachedImage (const std::string& image_id)
{
    CEF_REQUIRE_UI_THREAD();
    DCHECK(!image_id.empty());
    
    ImageMap::const_iterator it = image_map_.find(image_id);
    if (it != image_map_.end())
        return it->second;
    
    return NULL;
}

// static
casper::cef3::client::browser::ImageCache::ImageType casper::cef3::client::browser::ImageCache::GetImageType (const std::string& path)
{
    std::string ext = casper::cef3::shared::browser::utils::file::GetFileExtension(path);
    if (ext.empty())
        return TYPE_NONE;
    
    std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
    if (ext == "png")
        return TYPE_PNG;
    if (ext == "jpg" || ext == "jpeg")
        return TYPE_JPEG;
    
    return TYPE_NONE;
}

void casper::cef3::client::browser::ImageCache::LoadMissing (const casper::cef3::client::browser::ImageCache::ImageInfoSet& image_info,
                                                             const casper::cef3::client::browser::ImageCache::ImageSet& images,
                                                             const casper::cef3::client::browser::ImageCache::LoadImagesCallback& callback)
{
    CEF_REQUIRE_FILE_THREAD();
    
    DCHECK_EQ(image_info.size(), images.size());
    
    casper::cef3::client::browser::ImageCache::ImageContentSet contents;
    
    ImageInfoSet::const_iterator it1 = image_info.begin();
    ImageSet::const_iterator it2 = images.begin();
    for (; it1 != image_info.end() && it2 != images.end(); ++it1, ++it2) {
        const ImageInfo& info = *it1;
        casper::cef3::client::browser::ImageCache::ImageContent content;
        if (*it2 || info.id_ == kEmptyId) {
            // Image already exists or is intentionally empty.
            content.image_ = *it2;
        } else {
            LoadImageContents(info, &content);
        }
        contents.push_back(content);
    }
    
    CefPostTask(TID_UI, base::Bind(&ImageCache::UpdateCache, this, image_info,
                                   contents, callback));
}

// static
bool casper::cef3::client::browser::ImageCache::LoadImageContents (const casper::cef3::client::browser::ImageCache::ImageInfo& info,
                                                                   casper::cef3::client::browser::ImageCache::ImageContent* content)
{
    CEF_REQUIRE_FILE_THREAD();
    
    ImageRepSet::const_iterator it = info.reps_.begin();
    for (; it != info.reps_.end(); ++it) {
        const ImageRep& rep = *it;
        ImageType rep_type;
        std::string rep_contents;
        if (!LoadImageContents(rep.path_, info.internal_, &rep_type,
                               &rep_contents)) {
            LOG(ERROR) << "Failed to load image " << info.id_ << " from path "
            << rep.path_;
            return false;
        }
        content->contents_.push_back(
                                     casper::cef3::client::browser::ImageCache::ImageContent::RepContent(rep_type, rep.scale_factor_, rep_contents));
    }
    
    return true;
}

// static
bool casper::cef3::client::browser::ImageCache::LoadImageContents (const std::string& path, bool internal,
                                                                   casper::cef3::client::browser::ImageCache::ImageType* type, std::string* contents)
{
    CEF_REQUIRE_FILE_THREAD();
    
    *type = GetImageType(path);
    if ( *type == TYPE_NONE ) {
        return false;
    }
    
    if ( true == internal ) {
        if ( false == casper::cef3::shared::browser::utils::resource::LoadBinaryResource(path.c_str(), *contents) ) {
            return false;
        }
    } else if ( false == casper::cef3::shared::browser::utils::file::ReadFileToString(path, contents) ) {
        return false;
    }
    
    return !contents->empty();
}

void casper::cef3::client::browser::ImageCache::UpdateCache (const casper::cef3::client::browser::ImageCache::ImageInfoSet& image_info,
                                                             const casper::cef3::client::browser::ImageCache::ImageContentSet& contents,
                                                             const casper::cef3::client::browser::ImageCache::LoadImagesCallback& callback)
{
    CEF_REQUIRE_UI_THREAD();
    
    DCHECK_EQ(image_info.size(), contents.size());
    
    ImageSet images;
    
    casper::cef3::client::browser::ImageCache::ImageInfoSet::const_iterator it1 = image_info.begin();
    casper::cef3::client::browser::ImageCache::ImageContentSet::const_iterator it2 = contents.begin();
    for (; it1 != image_info.end() && it2 != contents.end(); ++it1, ++it2) {
        const casper::cef3::client::browser::ImageCache::ImageInfo&    info    = *it1;
        const casper::cef3::client::browser::ImageCache::ImageContent& content = *it2;
        if (content.image_ || info.id_ == kEmptyId) {
            // Image already exists or is intentionally empty.
            images.push_back(content.image_);
        } else {
            CefRefPtr<CefImage> image = CreateImage(info.id_, content);
            images.push_back(image);
            
            // Add the image to the map.
            image_map_.insert(std::make_pair(info.id_, image));
        }
    }
    
    callback.Run(images);
}

// static
CefRefPtr<CefImage> casper::cef3::client::browser::ImageCache::CreateImage (const std::string& image_id,
                                                                            const casper::cef3::client::browser::ImageCache::ImageContent& content)
{
    CEF_REQUIRE_UI_THREAD();
    
    // Shouldn't be creating an image if one already exists.
    DCHECK(!content.image_);
    
    if (content.contents_.empty())
        return NULL;
    
    CefRefPtr<CefImage> image = CefImage::CreateImage();
    
    casper::cef3::client::browser::ImageCache::ImageContent::RepContentSet::const_iterator it = content.contents_.begin();
    for (; it != content.contents_.end(); ++it) {
        const casper::cef3::client::browser::ImageCache::ImageContent::RepContent& rep = *it;
        if (rep.type_ == TYPE_PNG) {
            if (!image->AddPNG(rep.scale_factor_, rep.contents_.c_str(),
                               rep.contents_.size())) {
                LOG(ERROR) << "Failed to create image " << image_id << " for PNG@"
                << rep.scale_factor_;
                return NULL;
            }
        } else if (rep.type_ == TYPE_JPEG) {
            if (!image->AddJPEG(rep.scale_factor_, rep.contents_.c_str(),
                                rep.contents_.size())) {
                LOG(ERROR) << "Failed to create image " << image_id << " for JPG@"
                << rep.scale_factor_;
                return NULL;
            }
        } else {
            NOTREACHED();
            return NULL;
        }
    }
    
    return image;
}

