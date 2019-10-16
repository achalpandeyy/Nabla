// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_VIDEO_NULL_H_INCLUDED__
#define __C_VIDEO_NULL_H_INCLUDED__

#include "IVideoDriver.h"
#include "IFileSystem.h"
#include "irr/asset/IMesh.h"
#include "irr/video/IGPUMeshBuffer.h"
#include "IMeshSceneNode.h"
#include "CFPSCounter.h"
#include "SExposedVideoData.h"
#include "FW_Mutex.h"


#ifdef _MSC_VER
#pragma warning( disable: 4996)
#endif

namespace irr
{
namespace io
{
	class IWriteFile;
	class IReadFile;
} // end namespace io
namespace video
{
	class IImageLoader;
	class IImageWriter;

	class CNullDriver : public IVideoDriver
	{
    protected:
		//! destructor
		virtual ~CNullDriver();

	public:
        static FW_AtomicCounter ReallocationCounter;
        static int32_t incrementAndFetchReallocCounter();

		//! constructor
		CNullDriver(IrrlichtDevice* dev, io::IFileSystem* io, const core::dimension2d<uint32_t>& screenSize);

        inline bool isAllowedBufferViewFormat(asset::E_FORMAT _fmt) const override { return false; }
        inline bool isAllowedVertexAttribFormat(asset::E_FORMAT _fmt) const override { return false; }
        inline bool isColorRenderableFormat(asset::E_FORMAT _fmt) const override { return false; }
        inline bool isAllowedImageStoreFormat(asset::E_FORMAT _fmt) const override { return false; }
        inline bool isAllowedTextureFormat(asset::E_FORMAT _fmt) const override { return false; }
        inline bool isHardwareBlendableFormat(asset::E_FORMAT _fmt) const override { return false; }

        bool bindGraphicsPipeline(video::IGPURenderpassIndependentPipeline* _gpipeline) override { return false; }

        bool bindDescriptorSets(E_PIPELINE_BIND_POINT _pipelineType, const IGPUPipelineLayout* _layout,
            uint32_t _first, uint32_t _count, const IGPUDescriptorSet** _descSets, core::smart_refctd_dynamic_array<uint32_t>* _dynamicOffsets) override 
        { 
            return false; 
        }

		//!
        virtual bool initAuxContext() {return false;}
        virtual bool deinitAuxContext() {return false;}

		virtual bool beginScene(bool backBuffer=true, bool zBuffer=true,
				SColor color=SColor(255,0,0,0),
				const SExposedVideoData& videoData=SExposedVideoData(),
				core::rect<int32_t>* sourceRect=0);

		virtual bool endScene();

		//!
		virtual void issueGPUTextureBarrier() {}

		//! sets transformation
		virtual void setTransform(const E_4X3_TRANSFORMATION_STATE& state, const core::matrix4x3& mat);
		virtual void setTransform(const E_PROJECTION_TRANSFORMATION_STATE& state, const core::matrix4SIMD& mat);

        //! GPU fence, is signalled when preceeding GPU work is completed
        virtual core::smart_refctd_ptr<IDriverFence> placeFence(const bool& implicitFlushWaitSameThread=false) {return nullptr;}

        core::smart_refctd_ptr<ITexture> createGPUTexture(const ITexture::E_TEXTURE_TYPE& type, const uint32_t* size, uint32_t mipmapLevels, asset::E_FORMAT format = asset::EF_B8G8R8A8_UNORM) override;

		//! A.
        virtual E_MIP_CHAIN_ERROR validateMipChain(const ITexture* tex, const core::vector<asset::CImageData*>& mipChain)
        {
            if (!tex)
                return EMCE_OTHER_ERR;

            if (mipChain.size()==0)
                return EMCE_NO_ERR;

            for (core::vector<asset::CImageData*>::const_iterator it = mipChain.begin(); it!=mipChain.end(); it++)
            {
                asset::CImageData* img = *it;
                if (!img)
                    return EMCE_INVALID_IMAGE;

                const uint32_t mipLevel = img->getSupposedMipLevel();
                if (mipLevel>=tex->getMipMapLevelCount())
                    return EMCE_MIP_LEVEL_OUT_OF_BOUND;

                core::vector3d<uint32_t> textureSizeAtThisMipLevel = *reinterpret_cast< const core::vector3d<uint32_t>* >(tex->getSize());
                textureSizeAtThisMipLevel /= core::vector3d<uint32_t>(0x1u<<mipLevel);
                if (textureSizeAtThisMipLevel.X==0)
                    textureSizeAtThisMipLevel.X = 1;
                if (textureSizeAtThisMipLevel.Y==0)
                    textureSizeAtThisMipLevel.Y = 1;
                if (textureSizeAtThisMipLevel.Z==0)
                    textureSizeAtThisMipLevel.Z = 1;

                core::aabbox3d<uint32_t> imgCube(core::vector3d<uint32_t>(img->getSliceMin()),core::vector3d<uint32_t>(img->getSliceMax()));
                if (!imgCube.isFullInside(core::aabbox3d<uint32_t>(core::vector3d<uint32_t>(0,0,0),textureSizeAtThisMipLevel)))
                    return EMCE_SUB_IMAGE_OUT_OF_BOUNDS;
            }

            return EMCE_NO_ERR;
        }

		//! Sets new multiple render targets.
		virtual bool setRenderTarget(IFrameBuffer* frameBuffer, bool setNewViewport=true);

		//! Clears the ZBuffer.
		/** Note that you usually need not to call this method, as it
		is automatically done in IVideoDriver::beginScene() or
		IVideoDriver::setRenderTarget() if you enable zBuffer. But if
		you have to render some special things, you can clear the
		zbuffer during the rendering process with this method any time.
		*/
		virtual void clearZBuffer(const float &depth=0.0);

		virtual void clearStencilBuffer(const int32_t &stencil);

		virtual void clearZStencilBuffers(const float &depth, const int32_t &stencil);

		virtual void clearColorBuffer(const E_FBO_ATTACHMENT_POINT &attachment, const int32_t* vals);
		virtual void clearColorBuffer(const E_FBO_ATTACHMENT_POINT &attachment, const uint32_t* vals);
		virtual void clearColorBuffer(const E_FBO_ATTACHMENT_POINT &attachment, const float* vals);

		virtual void clearScreen(const E_SCREEN_BUFFERS &buffer, const float* vals);
		virtual void clearScreen(const E_SCREEN_BUFFERS &buffer, const uint32_t* vals);


		//! sets a viewport
		virtual void setViewPort(const core::rect<int32_t>& area);

		//! gets the area of the current viewport
		virtual const core::rect<int32_t>& getViewPort() const;

        virtual void drawMeshBuffer(const video::IGPUMeshBuffer* mb);

		//! Indirect Draw
		virtual void drawArraysIndirect( const asset::IMeshDataFormatDesc<video::IGPUBuffer>* vao,
                                         const asset::E_PRIMITIVE_TYPE& mode,
                                         const IGPUBuffer* indirectDrawBuff,
                                         const size_t& offset, const size_t& count, const size_t& stride);
		virtual void drawIndexedIndirect(const asset::IMeshDataFormatDesc<video::IGPUBuffer>* vao,
                                         const asset::E_PRIMITIVE_TYPE& mode,
                                         const asset::E_INDEX_TYPE& type,
                                         const IGPUBuffer* indirectDrawBuff,
                                         const size_t& offset, const size_t& count, const size_t& stride);

		//! get color format of the current color buffer
		virtual asset::E_FORMAT getColorFormat() const;

		//! get screen size
		virtual const core::dimension2d<uint32_t>& getScreenSize() const;

		//! get render target size
		virtual const core::dimension2d<uint32_t>& getCurrentRenderTargetSize() const;

		// get current frames per second value
		virtual int32_t getFPS() const;

		//! returns amount of primitives (mostly triangles) were drawn in the last frame.
		//! very useful method for statistics.
		virtual uint32_t getPrimitiveCountDrawn( uint32_t param = 0 ) const;

		//! \return Returns the name of the video driver. Example: In case of the DIRECT3D8
		//! driver, it would return "Direct3D8.1".
		virtual const wchar_t* getName() const;

		virtual void removeFrameBuffer(IFrameBuffer* framebuf);

		virtual void removeAllFrameBuffers();

		virtual void blitRenderTargets(IFrameBuffer* in, IFrameBuffer* out,
                                        bool copyDepth=true, bool copyStencil=true,
										core::recti srcRect=core::recti(0,0,0,0),
										core::recti dstRect=core::recti(0,0,0,0),
										bool bilinearFilter=false);

		//! Returns the maximum amount of primitives (mostly vertices) which
		//! the device is able to render with one drawIndexedTriangleList
		//! call.
		virtual uint32_t getMaximalIndicesCount() const;

		//! Enables or disables a texture creation flag.
		virtual void setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled);

		//! Returns if a texture creation flag is enabled or disabled.
		virtual bool getTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag) const;


	public:
		virtual void beginQuery(IQueryObject* query);
		virtual void endQuery(IQueryObject* query);
		virtual void beginQuery(IQueryObject* query, const size_t& index);
		virtual void endQuery(IQueryObject* query, const size_t& index);

		//! Only used by the engine internally.
		/** Used to notify the driver that the window was resized. */
		virtual void OnResize(const core::dimension2d<uint32_t>& size);

		//! Returns driver and operating system specific data about the IVideoDriver.
		virtual const SExposedVideoData& getExposedVideoData();

		//! Returns type of video driver
		virtual E_DRIVER_TYPE getDriverType() const;

		//! Returns the transformation set by setTransform
		virtual const core::matrix4x3& getTransform(const E_4X3_TRANSFORMATION_STATE& state);

		virtual const core::matrix4SIMD& getTransform(const E_PROJECTION_TRANSFORMATION_STATE& state);

        //virtual int32_t addHighLevelShaderMaterial(
        //    const char* vertexShaderProgram,
        //    const char* controlShaderProgram,
        //    const char* evaluationShaderProgram,
        //    const char* geometryShaderProgram,
        //    const char* pixelShaderProgram,
        //    uint32_t patchVertices=3,
        //    E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
        //    IShaderConstantSetCallBack* callback = 0,
        //    const char** xformFeedbackOutputs = NULL,
        //    const uint32_t& xformFeedbackOutputCount = 0,
        //    int32_t userData = 0,
        //    const char* vertexShaderEntryPointName="main",
        //    const char* controlShaderEntryPointName = "main",
        //    const char* evaluationShaderEntryPointName = "main",
        //    const char* geometryShaderEntryPointName = "main",
        //    const char* pixelShaderEntryPointName="main" );

        //virtual int32_t addHighLevelShaderMaterialFromFiles(
        //    const io::path& vertexShaderProgramFileName,
        //    const io::path& controlShaderProgramFileName,
        //    const io::path& evaluationShaderProgramFileName,
        //    const io::path& geometryShaderProgramFileName,
        //    const io::path& pixelShaderProgramFileName,
        //    uint32_t patchVertices=3,
        //    E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
        //    IShaderConstantSetCallBack* callback = 0,
        //    const char** xformFeedbackOutputs = NULL,
        //    const uint32_t& xformFeedbackOutputCount = 0,
        //    int32_t userData = 0,
        //    const char* vertexShaderEntryPointName="main",
        //    const char* controlShaderEntryPointName = "main",
        //    const char* evaluationShaderEntryPointName = "main",
        //    const char* geometryShaderEntryPointName = "main",
        //    const char* pixelShaderEntryPointName="main");

        //virtual int32_t addHighLevelShaderMaterialFromFiles(
        //    io::IReadFile* vertexShaderProgram,
        //    io::IReadFile* controlShaderProgram,
        //    io::IReadFile* evaluationShaderProgram,
        //    io::IReadFile* geometryShaderProgram,
        //    io::IReadFile* pixelShaderProgram,
        //    uint32_t patchVertices=3,
        //    E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
        //    IShaderConstantSetCallBack* callback = 0,
        //    const char** xformFeedbackOutputs = NULL,
        //    const uint32_t& xformFeedbackOutputCount = 0,
        //    int32_t userData = 0,
        //    const char* vertexShaderEntryPointName="main",
        //    const char* controlShaderEntryPointName = "main",
        //    const char* evaluationShaderEntryPointName = "main",
        //    const char* geometryShaderEntryPointName = "main",
        //    const char* pixelShaderEntryPointName="main");

        //virtual bool replaceHighLevelShaderMaterial(const int32_t &materialIDToReplace,
        //    const char* vertexShaderProgram,
        //    const char* controlShaderProgram,
        //    const char* evaluationShaderProgram,
        //    const char* geometryShaderProgram,
        //    const char* pixelShaderProgram,
        //    uint32_t patchVertices=3,
        //    E_MATERIAL_TYPE baseMaterial=video::EMT_SOLID,
        //    IShaderConstantSetCallBack* callback=0,
        //    const char** xformFeedbackOutputs = NULL,
        //    const uint32_t& xformFeedbackOutputCount = 0,
        //    int32_t userData=0,
        //    const char* vertexShaderEntryPointName="main",
        //    const char* controlShaderEntryPointName="main",
        //    const char* evaluationShaderEntryPointName="main",
        //    const char* geometryShaderEntryPointName="main",
        //    const char* pixelShaderEntryPointName="main");

		//! Enable/disable a clipping plane.
		//! There are at least 6 clipping planes available for the user to set at will.
		//! \param index: The plane index. Must be between 0 and MaxUserClipPlanes.
		//! \param enable: If true, enable the clipping plane else disable it.
		virtual void enableClipPlane(uint32_t index, bool enable);

		//! Returns the graphics card vendor name.
		virtual std::string getVendorInfo() {return "Not available on this driver.";}

		//! Returns the maximum texture size supported.
		virtual const uint32_t* getMaxTextureSize(const ITexture::E_TEXTURE_TYPE& type) const;

		//!
		virtual uint32_t getRequiredUBOAlignment() const override {return 0u;}

		//!
		virtual uint32_t getRequiredSSBOAlignment() const override {return 0u;}

		//!
		virtual uint32_t getRequiredTBOAlignment() const override {return 0u;}

        virtual uint32_t getMaxComputeWorkGroupSize(uint32_t) const override { return 0u; }

        virtual uint64_t getMaxUBOSize() const override { return 0ull; }

        virtual uint64_t getMaxSSBOSize() const override { return 0ull; }

        virtual uint64_t getMaxTBOSizeInTexels() const override { return 0ull; }

        virtual uint64_t getMaxBufferSize() const override { return 0ull; }

        uint32_t getMaxUBOBindings() const override { return 0u; }
        uint32_t getMaxSSBOBindings() const override { return 0u; }
        uint32_t getMaxTextureBindings() const override { return 0u; }
        uint32_t getMaxTextureBindingsCompute() const override { return 0u; }
        uint32_t getMaxImageBindings() const override { return 0u; }

	protected:
		//! returns a device dependent texture from a software surface (IImage)
		//! THIS METHOD HAS TO BE OVERRIDDEN BY DERIVED DRIVERS WITH OWN TEXTURES
		virtual core::smart_refctd_ptr<video::ITexture> createDeviceDependentTexture(const ITexture::E_TEXTURE_TYPE& type, const uint32_t* size, uint32_t mipmapLevels, const io::path& name, asset::E_FORMAT format = asset::EF_B8G8R8A8_UNORM);

        void bindDescriptorSets_generic(const IGPUPipelineLayout* _newLayout, uint32_t _first, uint32_t _count, const IGPUDescriptorSet** _descSets,
            const IGPUPipelineLayout** _destPplnLayouts);

		// prints renderer version
		void printVersion();

    protected:
		struct SSurface
		{
			video::ITexture* Surface;

			bool operator < (const SSurface& other) const
			{
			    return Surface->getName()<other.Surface->getName();
			    /*
			    int res = strcmp(Surface->getName().getInternalName().c_str(),other.Surface->getName().getInternalName().c_str());
			    if (res<0)
                    return true;
                else if (res>0)
                    return false;
                else
                    return Surface < other.Surface;
                */
			}
		};

		class SDummyTexture : public ITexture
		{
                _IRR_INTERFACE_CHILD(SDummyTexture) {}

                core::dimension2d<uint32_t> size;
		    public:
                SDummyTexture(const io::path& name) : ITexture(IDriverMemoryBacked::SDriverMemoryRequirements{{0,0,0},0,0,0,0},name), size(0,0)
                {
                }

                //special override as this object is always placement new'ed
                static inline void operator delete(void* ptr) noexcept
                {
                    return;
                }

                virtual E_DIMENSION_COUNT getDimensionality() const {return EDC_TWO;}
                virtual E_TEXTURE_TYPE getTextureType() const {return ETT_2D;}
                virtual E_VIRTUAL_TEXTURE_TYPE getVirtualTextureType() const {return EVTT_OPAQUE_FILTERABLE;}
                virtual const uint32_t* getSize() const { return &size.Width; }
                virtual uint32_t getMipMapLevelCount() const {return 1;}
                virtual core::dimension2du getRenderableSize() const { return size; }
                virtual E_DRIVER_TYPE getDriverType() const { return video::EDT_NULL; }
                virtual asset::E_FORMAT getColorFormat() const { return asset::EF_A1R5G5B5_UNORM_PACK16; }
                virtual core::rational<uint32_t> getPitch() const { return {0u,1u}; }
                virtual void regenerateMipMapLevels() {}
                virtual bool updateSubRegion(const asset::E_FORMAT &inDataColorFormat, const void* data, const uint32_t* minimum, const uint32_t* maximum, int32_t mipmap=0, const uint32_t& unpackRowByteAlignment=0) {return false;}
                virtual bool resize(const uint32_t* size, const uint32_t& mipLevels=0) {return false;}

                virtual IDriverMemoryAllocation* getBoundMemory() {return nullptr;}
                virtual const IDriverMemoryAllocation* getBoundMemory() const {return nullptr;}
                virtual size_t getBoundMemoryOffset() const {return 0u;}
		};

        IQueryObject* currentQuery[EQOT_COUNT];

		io::IFileSystem* FileSystem;

		core::rect<int32_t> ViewPort;
		core::dimension2d<uint32_t> ScreenSize;

		uint32_t matrixModifiedBits;
		core::matrix4SIMD ProjectionMatrices[EPTS_COUNT];
		core::matrix4x3 TransformationMatrices[E4X3TS_COUNT];

		CFPSCounter FPSCounter;

		uint32_t PrimitivesDrawn;

		uint32_t TextureCreationFlags;

		SExposedVideoData ExposedData;

		bool OverrideMaterial2DEnabled;

		uint32_t MaxTextureSizes[ITexture::ETT_COUNT][3];
	};

} // end namespace video
} // end namespace irr


#endif
