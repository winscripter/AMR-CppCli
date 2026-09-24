// Copyright (c) 2023-2026, winscripter


#include "flat.h"


extern "C" {


	//
	// Random 64-bit hex string used as a magic to make silent
	// processing of corrupted or invalid memory extremely rare.
	//
#define MAGIC64 0xA7F3C91E2B8D04FA


//
// Describes the type of the AMR internal structure.
//
	typedef enum _AMR_STRUCT_TYPE
	{
		//
		// Structure is an AMR-NB encoder.
		//
		STRUCT_TYPE_NARROWBAND_ENCODER,

		//
		// Structure is an AMR-NB decoder.
		//
		STRUCT_TYPE_NARROWBAND_DECODER,

		//
		// Structure is an AMR-WB encoder.
		//
		STRUCT_TYPE_WIDEBAND_ENCODER,

		//
		// Structure is an AMR-WB decoder.
		//
		STRUCT_TYPE_WIDEBAND_DECODER
	}
	AMR_STRUCT_TYPE;


	//
	// Internal AMR state structure.
	//
	typedef struct _AMR_INTERNAL_STRUCTURE
	{
		//
		// Identifies the type of this structure.
		//
		AMR_STRUCT_TYPE StructureType;

		//
		// Makes processing corrupted memory less likely.
		//
		ULONGLONG Magic64;

		//
		// Internal codec data.
		//
		PVOID PrivateData;

		//
		// DTX mode enabled?
		//
		DWORD UseDtx;
	}
	AMR_INTERNAL_STRUCTURE,
		* PAMR_INTERNAL_STRUCTURE;


	//
	// Internally creates the AMR state structure.
	// 
	// StructureType: The type of the AMR structure.
	// 
	// pvPrivateData: The actual AMR codec state.
	//
	PVOID
		AmrpAllocateInternalStructure(
			AMR_STRUCT_TYPE StructureType,
			PVOID pvPrivateData
		)
	{
		//
		// Result value
		//
		PAMR_INTERNAL_STRUCTURE pInternalStructure;

		//
		// Allocating through the process heap.
		//
		pInternalStructure = (PAMR_INTERNAL_STRUCTURE)
			HeapAlloc(
				GetProcessHeap(),
				0,
				sizeof(AMR_INTERNAL_STRUCTURE)
			);

		//
		// Init
		//
		pInternalStructure->StructureType = StructureType;
		pInternalStructure->Magic64 = MAGIC64;
		pInternalStructure->PrivateData = pvPrivateData;

		//
		// Ready.
		//
		return (pInternalStructure);
	}


	//
	// Internally creates the AMR state structure.
	// 
	// StructureType: The type of the AMR structure.
	// 
	// pvPrivateData: The actual AMR codec state.
	// 
	// bEnableDtx : Enable DTX mode?
	//
	PVOID
		AmrpAllocateInternalStructure2(
			AMR_STRUCT_TYPE StructureType,
			PVOID pvPrivateData,
			BOOL bEnableDtx
		)
	{
		PVOID pvState;

		pvState = AmrpAllocateInternalStructure(StructureType, pvPrivateData);

		((PAMR_INTERNAL_STRUCTURE)pvState)->UseDtx = bEnableDtx;
		return pvState;
	}


	//
	// Initializes a new AMR-NB encoder.
	// 
	// bEnableDtx : Enable DTX mode?
	//
	PVOID
		FLATAPI
		AmrCreateEncoderNarrow(
			BOOL bEnableDtx
		)
	{
		PVOID pvState;

		pvState = Encoder_Interface_init(bEnableDtx);

		return AmrpAllocateInternalStructure2(
			STRUCT_TYPE_NARROWBAND_ENCODER,
			pvState,
			bEnableDtx
		);
	}


	//
	// Initializes a new AMR-NB decoder.
	//
	PVOID
		FLATAPI
		AmrCreateDecoderNarrow()
	{
		PVOID pvState;

		pvState = Decoder_Interface_init();

		return AmrpAllocateInternalStructure(
			STRUCT_TYPE_NARROWBAND_DECODER,
			pvState
		);
	}


	//
	// Initializes a new AMR-WB encoder.
	// 
	// bEnableDtx : Enable DTX mode?
	//
	PVOID
		FLATAPI
		AmrCreateEncoderWide(
			BOOL bEnableDtx
		)
	{
		PVOID pvState;

		pvState = E_IF_init();

		return AmrpAllocateInternalStructure2(
			STRUCT_TYPE_WIDEBAND_ENCODER,
			pvState,
			bEnableDtx
		);
	}


	//
	// Initializes a new AMR-WB decoder.
	//
	PVOID
		FLATAPI
		AmrCreateDecoderWide()
	{
		PVOID pvState;

		pvState = D_IF_init();

		return AmrpAllocateInternalStructure(
			STRUCT_TYPE_WIDEBAND_DECODER,
			pvState
		);
	}


	//
	// Releases memory used by this AMR state.
	// After this, the AMR state is no longer usable.
	// 
	// pvState : The state to clear.
	//
	BOOL
		FLATAPI
		AmrClose(
			PVOID pvState
		)
	{
		PAMR_INTERNAL_STRUCTURE pInternalStruct;

		pInternalStruct = (PAMR_INTERNAL_STRUCTURE)pvState;

		if (pInternalStruct->Magic64 != MAGIC64)
		{
			//
			// Rejecting likely corrupted data.
			//
			return FALSE;
		}

		if (pInternalStruct->PrivateData == NULL)
		{
			return FALSE;
		}

		if (pInternalStruct->StructureType != STRUCT_TYPE_NARROWBAND_DECODER &&
			pInternalStruct->StructureType != STRUCT_TYPE_NARROWBAND_ENCODER &&
			pInternalStruct->StructureType != STRUCT_TYPE_WIDEBAND_DECODER &&
			pInternalStruct->StructureType != STRUCT_TYPE_WIDEBAND_ENCODER)
		{
			//
			// Unknown structure type
			//
			return FALSE;
		}

		//
		// Deallocation logic happens here.
		//

		switch (pInternalStruct->StructureType)
		{
		case STRUCT_TYPE_NARROWBAND_DECODER:
			Decoder_Interface_exit(pInternalStruct->PrivateData);
			break;
		case STRUCT_TYPE_NARROWBAND_ENCODER:
			Encoder_Interface_exit(pInternalStruct->PrivateData);
			break;
		case STRUCT_TYPE_WIDEBAND_DECODER:
			D_IF_exit(pInternalStruct->PrivateData);
			break;
		case STRUCT_TYPE_WIDEBAND_ENCODER:
			E_IF_exit(pInternalStruct->PrivateData);
			break;
		}

		pInternalStruct->PrivateData = NULL;

		//
		// Destroying the magic makes this structure no longer
		// valid.
		//
		pInternalStruct->Magic64 = 0;

		HeapFree(GetProcessHeap(), 0, pInternalStruct);

		return TRUE;
	}


	//
	// Checks if the state represents a wideband codec.
	// 
	// pvState : AMR state
	//
	BOOL
		FLATAPI
		AmrIsWideband(
			PVOID pvState
		)
	{
		if (((PAMR_INTERNAL_STRUCTURE)pvState)->Magic64 != MAGIC64)
		{
			return FALSE;
		}

		return ((PAMR_INTERNAL_STRUCTURE)pvState)->StructureType == STRUCT_TYPE_WIDEBAND_DECODER ||
			((PAMR_INTERNAL_STRUCTURE)pvState)->StructureType == STRUCT_TYPE_WIDEBAND_ENCODER;
	}


	//
	// Checks if the state represents a narrowband codec.
	// 
	// pvState : AMR state
	//
	BOOL
		FLATAPI
		AmrIsNarrowband(
			PVOID pvState
		)
	{
		if (((PAMR_INTERNAL_STRUCTURE)pvState)->Magic64 != MAGIC64)
		{
			return FALSE;
		}

		return ((PAMR_INTERNAL_STRUCTURE)pvState)->StructureType == STRUCT_TYPE_NARROWBAND_DECODER ||
			((PAMR_INTERNAL_STRUCTURE)pvState)->StructureType == STRUCT_TYPE_NARROWBAND_ENCODER;
	}


	//
	// Checks if the state represents an encoder.
	// 
	// pvState : AMR state
	//
	BOOL
		FLATAPI
		AmrIsEncoder(
			PVOID pvState
		)
	{
		if (((PAMR_INTERNAL_STRUCTURE)pvState)->Magic64 != MAGIC64)
		{
			return FALSE;
		}

		return ((PAMR_INTERNAL_STRUCTURE)pvState)->StructureType == STRUCT_TYPE_WIDEBAND_ENCODER ||
			((PAMR_INTERNAL_STRUCTURE)pvState)->StructureType == STRUCT_TYPE_NARROWBAND_ENCODER;
	}


	//
	// Checks if the state represents a decoder.
	// 
	// pvState : AMR state
	//
	BOOL
		FLATAPI
		AmrIsDecoder(
			PVOID pvState
		)
	{
		if (((PAMR_INTERNAL_STRUCTURE)pvState)->Magic64 != MAGIC64)
		{
			return FALSE;
		}

		return ((PAMR_INTERNAL_STRUCTURE)pvState)->StructureType == STRUCT_TYPE_WIDEBAND_DECODER ||
			((PAMR_INTERNAL_STRUCTURE)pvState)->StructureType == STRUCT_TYPE_NARROWBAND_DECODER;
	}


	//
	// Encodes one AMR frame.
	//
	BOOL
		FLATAPI
		AmrEncode(
			PVOID pvState,
			PSHORT psSpeech,
			PBYTE pbSerial,
			DWORD dwMode,
			BOOL bForceSpeech
		)
	{
		PAMR_INTERNAL_STRUCTURE pInternalStructure;

		if (!AmrIsEncoder(pvState))
		{
			return FALSE;
		}

		pInternalStructure = (PAMR_INTERNAL_STRUCTURE)pvState;

		if (pInternalStructure->StructureType == STRUCT_TYPE_NARROWBAND_ENCODER)
		{
			return Encoder_Interface_Encode(
				pInternalStructure->PrivateData,
				(enum Mode)dwMode,
				psSpeech,
				pbSerial,
				bForceSpeech
			);
		}
		else
		{
			return E_IF_encode(
				pInternalStructure->PrivateData,
				(Word16)dwMode,
				psSpeech,
				pbSerial,
				pInternalStructure->UseDtx
			);
		}
	}


	//
	// Decodes one AMR frame.
	//
	BOOL
		FLATAPI
		AmrDecode(
			PVOID pvState,
			PBYTE pbSerial,
			PSHORT psSpeech
		)
	{
		PAMR_INTERNAL_STRUCTURE pInternalStructure;

		if (!AmrIsDecoder(pvState))
		{
			return FALSE;
		}

		pInternalStructure = (PAMR_INTERNAL_STRUCTURE)pvState;

		if (pInternalStructure->StructureType == STRUCT_TYPE_NARROWBAND_DECODER)
		{
			Decoder_Interface_Decode(
				pInternalStructure->PrivateData,
				pbSerial,
				psSpeech,
				0
			);

			return TRUE;
		}
		else
		{
			D_IF_decode(
				pInternalStructure->PrivateData,
				pbSerial,
				psSpeech,
				0
			);

			return TRUE;
		}
	}

}
