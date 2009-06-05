/*
 * OpenClonk, http://www.openclonk.org
 *
 * Copyright (c) 1998-2000, 2007  Matthes Bender
 * Copyright (c) 2001-2002, 2005-2007  Sven Eberhardt
 * Copyright (c) 2007  Peter Wortmann
 * Copyright (c) 2001-2009, RedWolf Design GmbH, http://www.clonk.de
 *
 * Portions might be copyrighted by other authors who have contributed
 * to OpenClonk.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * See isc_license.txt for full license and disclaimer.
 *
 * "Clonk" is a registered trademark of Matthes Bender.
 * See clonk_trademark_license.txt for full license.
 */

/* Material definitions used by the landscape */

#ifndef INC_C4Material
#define INC_C4Material

#include <C4Id.h>
#include <C4Shape.h>
#include <C4Facet.h>
#include <vector>

#define C4MatOv_Default			0
#define C4MatOv_Exact				1
#define C4MatOv_None				2
#define C4MatOv_HugeZoom		4
#define C4MatOv_Monochrome	8

enum MaterialInteractionEvent
	{
	meePXSPos=0,  // PXS check before movement
	meePXSMove=1, // PXS movement
	meeMassMove=2, // MassMover-movement
	};

typedef bool (*C4MaterialReactionFunc)(struct C4MaterialReaction *pReaction, int32_t &iX, int32_t &iY, int32_t iLSPosX, int32_t iLSPosY, FIXED &fXDir, FIXED &fYDir, int32_t &iPxsMat, int32_t iLsMat, MaterialInteractionEvent evEvent, bool *pfPosChanged);

struct C4MaterialReaction
	{
	static inline bool NoReaction(struct C4MaterialReaction *pReaction, int32_t &iX, int32_t &iY, int32_t iLSPosX, int32_t iLSPosY, FIXED &fXDir, FIXED &fYDir, int32_t &iPxsMat, int32_t iLsMat, MaterialInteractionEvent evEvent, bool *pfPosChanged) { return false; }

	C4MaterialReactionFunc pFunc; // Guarantueed to be non-NULL
	bool fUserDefined;        // false for internal reactions generated by material parameters
	StdCopyStrBuf TargetSpec; // target material specification
	StdCopyStrBuf ScriptFunc; // for reaction func 'script': Script func to be called for reaction evaluation
#ifdef C4ENGINE
	C4AulFunc *pScriptFunc;   // resolved script function
#endif
	uint32_t iExecMask;       // execution mask: Bit mask with indices into MaterialInteractionEvent
	bool fReverse;            // if set, spec will be handled as if specified in target mat def
	bool fInverseSpec;        // if set, all mats except the given are used
	bool fInsertionCheck;     // if set, splash/slide checks are done prior to reaction execution
	int32_t iDepth;           // in mat conversion depth
	StdCopyStrBuf sConvertMat;// in mat conversion material (string)
	int32_t iConvertMat;      // in mat conversion material; evaluated in CrossMapMaterials
	int32_t iCorrosionRate;   // chance of doing a corrosion

#ifdef C4ENGINE
	C4MaterialReaction(C4MaterialReactionFunc pFunc) : pFunc(pFunc), fUserDefined(false), pScriptFunc(NULL), iExecMask(~0u), fReverse(false), fInverseSpec(false), fInsertionCheck(true), iDepth(0), iConvertMat(-1), iCorrosionRate(100) {}
	C4MaterialReaction() : pFunc(&NoReaction), fUserDefined(true), pScriptFunc(NULL), iExecMask(~0u), fReverse(false), fInverseSpec(false), fInsertionCheck(true), iDepth(0), iConvertMat(-1), iCorrosionRate(100) { }
#endif

	void CompileFunc(StdCompiler *pComp);

#ifdef C4ENGINE
	void ResolveScriptFuncs(const char *szMatName);
#endif

	bool operator ==(const C4MaterialReaction &rCmp) const { return false; } // never actually called; only comparing with empty vector of C4MaterialReactions
	};

class C4MaterialCore
  {
	public:
		C4MaterialCore();
		~C4MaterialCore() { Clear(); }

	public:
		std::vector<C4MaterialReaction> CustomReactionList;
  public:
    char Name[C4M_MaxName+1];
    uint32_t Color[C4M_ColsPerMat*3];
		uint32_t Alpha[C4M_ColsPerMat*2];
    int32_t  MapChunkType;
    int32_t  Density;
    int32_t  Friction;
    int32_t  DigFree;
    int32_t  BlastFree;
    C4ID Dig2Object;
    int32_t  Dig2ObjectRatio;
    int32_t  Dig2ObjectOnRequestOnly;
    C4ID Blast2Object;
    int32_t  Blast2ObjectRatio;
    int32_t  Blast2PXSRatio;
    int32_t  Instable;
    int32_t  MaxAirSpeed;
    int32_t  MaxSlide;
    int32_t  WindDrift;
    int32_t  Inflammable;
    int32_t  Incindiary;
    int32_t  Extinguisher;
		int32_t  Corrosive;
		int32_t  Corrode;
    int32_t  Soil;
		int32_t  Placement; // placement order for landscape shading
		StdCopyStrBuf sTextureOverlay; // overlayed texture for this material
		int32_t  OverlayType;	// defines the way in which the overlay texture is applied
		StdCopyStrBuf sPXSGfx;			// newgfx: picture used for loose pxs
		C4TargetRect PXSGfxRt;					// newgfx: facet rect of pixture used for loose pixels
		int32_t  PXSGfxSize;
    StdCopyStrBuf sBlastShiftTo;
    StdCopyStrBuf sInMatConvert;
    StdCopyStrBuf sInMatConvertTo;
		int32_t  InMatConvertDepth;         // material converts only if it finds the same material above
    int32_t  BelowTempConvert;
    int32_t  BelowTempConvertDir;
    StdCopyStrBuf sBelowTempConvertTo;
    int32_t  AboveTempConvert;
    int32_t  AboveTempConvertDir;
    StdCopyStrBuf sAboveTempConvertTo;
		int32_t  ColorAnimation;
		int32_t  TempConvStrength;
		int32_t  MinHeightCount; // minimum material thickness in order for it to be counted
    int32_t  SplashRate;
	public:
		void Clear();
		void Default();
		BOOL Load(C4Group &hGroup, const char *szEntryName);
		DWORD GetDWordColor(int32_t iIndex); // get indexed material color as DWord
		void CompileFunc(StdCompiler *pComp);
  };

class C4Material: public C4MaterialCore
  {
  public:
    C4Material();
  public:
		// Cross-mapped material values
    int32_t BlastShiftTo; // MatTex
    int32_t InMatConvertTo; // Mat
    int32_t BelowTempConvertTo; // MatTex
    int32_t AboveTempConvertTo; // MatTex
#ifdef C4ENGINE
		int32_t DefaultMatTex;			// texture used for single pixel values
		C4Facet PXSFace;				// loose pixel facet

		void UpdateScriptPointers(); // set all material script pointers
#endif
  };

class C4MaterialMap
  {
  public:
    C4MaterialMap();
    ~C4MaterialMap();
  public:
    int32_t Num;
    C4Material *Map;
		C4MaterialReaction **ppReactionMap;

		C4MaterialReaction DefReactConvert, DefReactPoof, DefReactCorrode, DefReactIncinerate, DefReactInsert;
	public:
		// default reactions
		static bool mrfConvert(C4MaterialReaction *pReaction, int32_t &iX, int32_t &iY, int32_t iLSPosX, int32_t iLSPosY, FIXED &fXDir, FIXED &fYDir, int32_t &iPxsMat, int32_t iLsMat, MaterialInteractionEvent evEvent, bool *pfPosChanged);
		static bool mrfPoof   (C4MaterialReaction *pReaction, int32_t &iX, int32_t &iY, int32_t iLSPosX, int32_t iLSPosY, FIXED &fXDir, FIXED &fYDir, int32_t &iPxsMat, int32_t iLsMat, MaterialInteractionEvent evEvent, bool *pfPosChanged);
		static bool mrfCorrode(C4MaterialReaction *pReaction, int32_t &iX, int32_t &iY, int32_t iLSPosX, int32_t iLSPosY, FIXED &fXDir, FIXED &fYDir, int32_t &iPxsMat, int32_t iLsMat, MaterialInteractionEvent evEvent, bool *pfPosChanged);
		static bool mrfIncinerate(C4MaterialReaction *pReaction, int32_t &iX, int32_t &iY, int32_t iLSPosX, int32_t iLSPosY, FIXED &fXDir, FIXED &fYDir, int32_t &iPxsMat, int32_t iLsMat, MaterialInteractionEvent evEvent, bool *pfPosChanged);
		static bool mrfInsert (C4MaterialReaction *pReaction, int32_t &iX, int32_t &iY, int32_t iLSPosX, int32_t iLSPosY, FIXED &fXDir, FIXED &fYDir, int32_t &iPxsMat, int32_t iLsMat, MaterialInteractionEvent evEvent, bool *pfPosChanged);
		// user-defined actions
#ifdef C4ENGINE
		static bool mrfScript(C4MaterialReaction *pReaction, int32_t &iX, int32_t &iY, int32_t iLSPosX, int32_t iLSPosY, FIXED &fXDir, FIXED &fYDir, int32_t &iPxsMat, int32_t iLsMat, MaterialInteractionEvent evEvent, bool *pfPosChanged);
#endif
  public:
	  void Default();
    void Clear();
    int32_t Load(C4Group &hGroup, C4Group* OverloadFile = 0);
		bool HasMaterials(C4Group &hGroup) const;
    int32_t Get(const char *szMaterial);
    BOOL SaveEnumeration(C4Group &hGroup);
	  BOOL LoadEnumeration(C4Group &hGroup);
		C4MaterialReaction *GetReactionUnsafe(int32_t iPXSMat, int32_t iLandscapeMat)
			{ assert(ppReactionMap); assert(Inside<int32_t>(iPXSMat,-1,Num-1)); assert(Inside<int32_t>(iLandscapeMat,-1,Num-1));
				return ppReactionMap[(iLandscapeMat+1)*(Num+1) + iPXSMat+1]; }
		C4MaterialReaction *GetReaction(int32_t iPXSMat, int32_t iLandscapeMat);
#ifdef C4ENGINE
		void UpdateScriptPointers(); // set all material script pointers
    void CrossMapMaterials();
#endif
  protected:
		void SetMatReaction(int32_t iPXSMat, int32_t iLSMat, C4MaterialReaction *pReact);
	  BOOL SortEnumeration(int32_t iMat, const char *szMatName);
  };

const int32_t C4M_Flat    = 0,
          C4M_TopFlat = 1,
          C4M_Smooth  = 2,
          C4M_Rough   = 3,

          // Material Density Levels

					C4M_Vehicle   = 100,
          C4M_Solid     = 50,
          C4M_SemiSolid = 25,
          C4M_Liquid    = 25,
					C4M_Background= 0;

const int32_t MNone = -1;

#endif
