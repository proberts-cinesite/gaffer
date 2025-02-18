//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "GafferScene/AttributeProcessor.h"

#include "Gaffer/StringPlug.h"
#include "Gaffer/TweakPlug.h"

#include "IECoreScene/ShaderNetwork.h"

namespace GafferScene
{

class GAFFERSCENE_API ShaderTweaks : public AttributeProcessor
{

	public :

		ShaderTweaks( const std::string &name=defaultName<ShaderTweaks>() );
		~ShaderTweaks() override;

		GAFFER_NODE_DECLARE_TYPE( GafferScene::ShaderTweaks, ShaderTweaksTypeId, AttributeProcessor );

		Gaffer::StringPlug *shaderPlug();
		const Gaffer::StringPlug *shaderPlug() const;

		Gaffer::BoolPlug *ignoreMissingPlug();
		const Gaffer::BoolPlug *ignoreMissingPlug() const;

		Gaffer::TweaksPlug *tweaksPlug();
		const Gaffer::TweaksPlug *tweaksPlug() const;

		Gaffer::BoolPlug *localisePlug();
		const Gaffer::BoolPlug *localisePlug() const;

	protected :

		bool affectsProcessedAttributes( const Gaffer::Plug *input ) const override;
		void hashProcessedAttributes( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		IECore::ConstCompoundObjectPtr computeProcessedAttributes( const ScenePath &path, const Gaffer::Context *context, const IECore::CompoundObject *inputAttributes ) const override;

		static size_t g_firstPlugIndex;

	private :

		bool applyTweaks( IECoreScene::ShaderNetwork *shaderNetwork, Gaffer::TweakPlug::MissingMode missingMode ) const;

};

IE_CORE_DECLAREPTR( ShaderTweaks )

} // namespace GafferScene
