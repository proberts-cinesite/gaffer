//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, John Haddon. All rights reserved.
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "GafferBindings/GraphComponentBinding.h"
#include "GafferBindings/Serialisation.h"

#include "Gaffer/ContextProcessor.h"
#include "Gaffer/Node.h"
#include "Gaffer/Switch.h"

#include "IECorePython/ExceptionAlgo.h"
#include "IECorePython/ScopedGILLock.h"

#include <utility>

namespace GafferBindings
{

template<typename T, typename TWrapper=T>
class NodeClass : public GraphComponentClass<T, TWrapper>
{
	public :

		NodeClass( const char *docString = nullptr );
		NodeClass( const char *docString, boost::python::no_init_t );

};

template<typename T>
class NodeWrapper : public GraphComponentWrapper<T>
{
	public :

		using WrappedType = T;

		template<typename... Args>
		NodeWrapper( PyObject *self, Args&&... args )
			:	GraphComponentWrapper<T>( self, std::forward<Args>( args )... )
		{
		}

		bool isInstanceOf( IECore::TypeId typeId ) const override
		{
			// Optimise for common queries for types we know about. The standard
			// wrapper implementation of `isInstanceOf()` would have to enter
			// Python just in case the type was implemented there. Entering
			// Python is incredibly costly for such a simple operation, and we
			// perform these operations often, so these optimisations are well
			// worth it.

			if(
				// We're a Node, so we cannot be a plug.
				typeId == (IECore::TypeId)Gaffer::PlugTypeId ||
				typeId == (IECore::TypeId)Gaffer::ValuePlugTypeId
			)
			{
				return false;
			}

			if(
				// It's important to optimise for ContextProcessor and
				// Switch specifically, because they are queried heavily during
				// the `Dispatcher::dispatch()` process.
				typeId == (IECore::TypeId)Gaffer::ContextProcessorTypeId ||
				typeId == (IECore::TypeId)Gaffer::SwitchTypeId ||
				// ScriptNode, DependencyNode and EditScope also appear on
				// performance critical code paths.
				typeId == (IECore::TypeId)Gaffer::ScriptNodeTypeId ||
				typeId == (IECore::TypeId)Gaffer::DependencyNodeTypeId ||
				typeId == (IECore::TypeId)Gaffer::EditScopeTypeId
			)
			{
				// The types above are implemented in C++, so there is no need
				// to consider Python overrides for `isInstanceOf()`. The base
				// class implementation is sufficient.
				return WrappedType::isInstanceOf( typeId );
			}

			return GraphComponentWrapper<T>::isInstanceOf( typeId );
		}

		bool acceptsInput( const Gaffer::Plug *plug, const Gaffer::Plug *inputPlug ) const override
		{
			if( this->isSubclassed() )
			{
				IECorePython::ScopedGILLock gilLock;
				try
				{
					boost::python::object f = this->methodOverride( "acceptsInput" );
					if( f )
					{
						return f( Gaffer::PlugPtr( const_cast<Gaffer::Plug *>( plug ) ), Gaffer::PlugPtr( const_cast<Gaffer::Plug *>( inputPlug ) ) );
					}
				}
				catch( const boost::python::error_already_set &e )
				{
					IECorePython::ExceptionAlgo::translatePythonException();
				}
			}
			return T::acceptsInput( plug, inputPlug );
		}

};

class GAFFERBINDINGS_API NodeSerialiser : public Serialisation::Serialiser
{

	public :

		IE_CORE_DECLAREMEMBERPTR( NodeSerialiser )

		void moduleDependencies( const Gaffer::GraphComponent *graphComponent, std::set<std::string> &modules, const Serialisation &serialisation ) const override;
		/// Implemented to serialise per-instance metadata.
		std::string postHierarchy( const Gaffer::GraphComponent *graphComponent, const std::string &identifier, Serialisation &serialisation ) const override;
		/// Implemented so that only plugs are serialised - child nodes are expected to
		/// be a part of the implementation of the node rather than something the user
		/// has created themselves.
		bool childNeedsSerialisation( const Gaffer::GraphComponent *child, const Serialisation &serialisation ) const override;
		/// Implemented so that dynamic plugs are constructed appropriately.
		bool childNeedsConstruction( const Gaffer::GraphComponent *child, const Serialisation &serialisation ) const override;

};

} // namespace GafferBindings

#include "GafferBindings/NodeBinding.inl"
