#ifndef MAYA_API_MTextureCache
#define MAYA_API_MTextureCache

//-
// ==========================================================================
// Copyright (C) 1995 - 2006 Autodesk, Inc. and/or its licensors.  All 
// rights reserved.
//
// The coded instructions, statements, computer programs, and/or related 
// material (collectively the "Data") in these files contain unpublished 
// information proprietary to Autodesk, Inc. ("Autodesk") and/or its 
// licensors, which is protected by U.S. and Canadian federal copyright 
// law and by international treaties.
//
// The Data is provided for use exclusively by You. You have the right 
// to use, modify, and incorporate this Data into other products for 
// purposes authorized by the Autodesk software license agreement, 
// without fee.
//
// The copyright notices in the Software and this entire statement, 
// including the above license grant, this restriction and the 
// following disclaimer, must be included in all copies of the 
// Software, in whole or in part, and all derivative works of 
// the Software, unless such copies or derivative works are solely 
// in the form of machine-executable object code generated by a 
// source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND. 
// AUTODESK DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED 
// WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF 
// NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
// PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE, OR 
// TRADE PRACTICE. IN NO EVENT WILL AUTODESK AND/OR ITS LICENSORS 
// BE LIABLE FOR ANY LOST REVENUES, DATA, OR PROFITS, OR SPECIAL, 
// DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK 
// AND/OR ITS LICENSORS HAS BEEN ADVISED OF THE POSSIBILITY 
// OR PROBABILITY OF SUCH DAMAGES.
//
// ==========================================================================
//+

// MTextureCache.h

///////////////////////////////////////////////////////////////////
// DESCRIPTION: Texture cache, used to temporarily store textures.
//				Eventually, this class will likely end up in the 
//				Maya API.
//
//				This class is not currently thread-safe.
//
// AUTHOR: Christian Laforte
//
///////////////////////////////////////////////////////////////////

#ifdef WIN32
#pragma warning( disable : 4786 )		// Disable stupid STL warnings.
#endif

#include <maya/MObject.h>
#include <map>
#include <string>
#include <algorithm>

#include "MTexture.h"
#include "NodeMonitor.h"


class MTextureCache;

class MTextureCacheElement
{
friend class MTextureCache;

public:
	MTextureCacheElement()
	{
		lastAccessedTimestamp = -1; 
		m_texture = NULL; 
	}
	
	~MTextureCacheElement();

	MTexture* texture() { return m_texture; }

private:
	MTexture* m_texture;
	unsigned int lastAccessedTimestamp;		// can be used to track when the texture was last used.
	NodeMonitor fMonitor;
};

// This class implements a singleton node with reference counting.
// The refcount starts with a value equal to 0. Everytime instance()
// gets called, the refcount is incremented by one. Everytime
// release() gets called, the refcount is decremented by one,
// and if following that the refcount value is 0, the texture cache
// singleton is destroyed.
class MTextureCache : public NodeMonitorManager
{
protected:
	MTextureCache()
	{
		m_currentTimestamp = 0;
	}

public:
	~MTextureCache();

	static MTextureCache* instance()
	{
		if (!m_instance)
		{
			m_instance = new MTextureCache;
		}

		refcount++;

		return m_instance;
	}

	static void release()
	{
		refcount--;

		if (refcount == 0 && m_instance)
		{
			delete m_instance;
			m_instance = NULL;
		}
	}

	static int getReferenceCount()
	{
		return refcount;
	}

	// Return a reference to the texture. There's no reference counting yet.
	MTexture* texture(MObject textureObj, 
				 MTexture::Type type = MTexture::RGBA, 
				 bool mipmapped = true,
 				 GLenum target = GL_TEXTURE_2D);

	// Returns true if the texture was found and bound; returns false otherwise.
	bool bind(MObject textureObj, 
			  MTexture::Type type = MTexture::RGBA, 
			  bool mipmapped = true,
  			  GLenum target = GL_TEXTURE_2D);

	void incrementTimestamp(unsigned int increment=1);

	// Called by a node monitor when the watched node is renamed.
	void onNodeRenamed(MObject& node, MString oldName, MString newName);

private:
	static int refcount;

	std::map<std::string, MTextureCacheElement*> m_textureTable;
	typedef std::map<std::string, MTextureCacheElement*> string_to_cacheElement_map;

	unsigned int m_currentTimestamp;

	static MTextureCache* m_instance;
};



#endif // MAYA_API_MTextureCache