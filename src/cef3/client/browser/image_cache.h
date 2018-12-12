// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_BROWSER_IMAGE_CACHE_H_
#define CASPER_CEF3_CLIENT_BROWSER_IMAGE_CACHE_H_
#pragma once

#include <map>
#include <vector>

#include "include/base/cef_bind.h"
#include "include/base/cef_ref_counted.h"
#include "include/cef_image.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace browser
            {
                
                // Simple image caching implementation.
                class ImageCache : public base::RefCountedThreadSafe<ImageCache, CefDeleteOnUIThread>
                {
                    
                private: // Data Type(s)
                                        
                    enum ImageType {
                        TYPE_NONE,
                        TYPE_PNG,
                        TYPE_JPEG,
                    };
                    
                    struct ImageContent {
                        
                        ImageContent() {}
                        
                        struct RepContent {
                            RepContent(ImageType type, float scale_factor, const std::string& contents)
                            : type_(type), scale_factor_(scale_factor), contents_(contents) {}
                            
                            ImageType type_;
                            float scale_factor_;
                            std::string contents_;
                        };
                        typedef std::vector<RepContent> RepContentSet;
                        RepContentSet contents_;
                        
                        CefRefPtr<CefImage> image_;
                    };
                    
                    typedef std::vector<ImageContent> ImageContentSet;
                    
                public:
                    ImageCache();
                    
                    // Image representation at a specific scale factor.
                    struct ImageRep {
                        ImageRep(const std::string& path, float scale_factor);
                        
                        // Full file system path.
                        std::string path_;
                        
                        // Image scale factor (usually 1.0f or 2.0f).
                        float scale_factor_;
                    };
                    typedef std::vector<ImageRep> ImageRepSet;
                    
                    
                    // Unique image that may have multiple representations.
                    struct ImageInfo {
                        ImageInfo(const std::string& id,
                                  const ImageRepSet& reps,
                                  bool internal,
                                  bool force_reload);
                        
                        // Helper for returning an empty image.
                        static ImageInfo Empty();
                        
                        // Helpers for creating common representations.
                        static ImageInfo Create1x(const std::string& id,
                                                  const std::string& path_1x,
                                                  bool internal);
                        static ImageInfo Create2x(const std::string& id,
                                                  const std::string& path_1x,
                                                  const std::string& path_2x,
                                                  bool internal);
                        static ImageInfo Create2x(const std::string& id);
                        
                        // Image unique ID.
                        std::string id_;
                        
                        // Image representations to load.
                        ImageRepSet reps_;
                        
                        // True if the image is internal (loaded via LoadBinaryResource).
                        bool internal_;
                        
                        // True to force reload.
                        bool force_reload_;
                    };
                    typedef std::vector<ImageInfo> ImageInfoSet;
                    
                    typedef std::vector<CefRefPtr<CefImage>> ImageSet;
                    
                    typedef base::Callback<void(const ImageSet& /*images*/)> LoadImagesCallback;
                    
                    // Loads the images represented by |image_info|. Executes |callback|
                    // either synchronously or asychronously on the UI thread after completion.
                    void LoadImages(const ImageInfoSet& image_info,
                                    const LoadImagesCallback& callback);
                    
                    // Returns an image that has already been cached. Must be called on the
                    // UI thread.
                    CefRefPtr<CefImage> GetCachedImage(const std::string& image_id);
                    
                private:
                    // Only allow deletion via scoped_refptr.
                    friend struct CefDeleteOnThread<TID_UI>;
                    friend class base::RefCountedThreadSafe<ImageCache, CefDeleteOnUIThread>;
                    
                    ~ImageCache();
                   
                    static ImageType GetImageType(const std::string& path);
                    
                    // Load missing image contents on the FILE thread.
                    void LoadMissing(const ImageInfoSet& image_info,
                                     const ImageSet& images,
                                     const LoadImagesCallback& callback);
                    static bool LoadImageContents(const ImageInfo& info, ImageContent* content);
                    static bool LoadImageContents(const std::string& path,
                                                  bool internal,
                                                  ImageType* type,
                                                  std::string* contents);
                    
                    // Create missing CefImage representations on the UI thread.
                    void UpdateCache(const ImageInfoSet& image_info,
                                     const ImageContentSet& contents,
                                     const LoadImagesCallback& callback);
                    static CefRefPtr<CefImage> CreateImage(const std::string& image_id,
                                                           const ImageContent& content);
                    
                    // Map image ID to image representation. Only accessed on the UI thread.
                    typedef std::map<std::string, CefRefPtr<CefImage>> ImageMap;
                    ImageMap image_map_;
                    
                }; // end of class 'ImageCache'

            } // end of namespace 'browser'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_BROWSER_IMAGE_CACHE_H_

