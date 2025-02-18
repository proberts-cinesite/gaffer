//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, John Haddon. All rights reserved.
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

#include "boost/python.hpp"

#include "ApplicationRootBinding.h"

#include "GafferBindings/NodeBinding.h"
#include "GafferBindings/Serialisation.h"
#include "GafferBindings/SignalBinding.h"

#include "Gaffer/ApplicationRoot.h"
#include "Gaffer/Preferences.h"

#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/ScopedGILLock.h"

#include <fstream>

using namespace boost::python;
using namespace GafferBindings;
using namespace Gaffer;

namespace
{

class ApplicationRootWrapper : public IECorePython::RunTimeTypedWrapper<ApplicationRoot>
{

	public :

		ApplicationRootWrapper( PyObject *self, const std::string &name = defaultName<ApplicationRoot>() )
			:	IECorePython::RunTimeTypedWrapper<ApplicationRoot>( self, name )
		{
		}

		void savePreferences( const std::filesystem::path &fileName ) const override
		{
			IECorePython::ScopedGILLock gilLock;

			// serialise everything
			Serialisation s( preferences(), "application.root()[\"preferences\"]" );

			// make the directory for the preferences file if it doesn't exist yet
			std::filesystem::create_directories( fileName.parent_path() );

			// then make the file and write the serialisation into it
			std::ofstream f;
			f.open( fileName.c_str() );
			if( !f.is_open() )
			{
				throw IECore::Exception( "Unable to open file \"" + fileName.string() + "\"" );
			}

			f << "# This file was automatically generated by Gaffer.\n";
			f << "# Do not edit this file - it will be overwritten.\n\n";
			f << s.result() << std::endl;

			if( !f.good() )
			{
				throw IECore::Exception( "Error while writing file \"" + fileName.string() + "\"" );
			}
		}

};

IECore::ObjectPtr getClipboardContents( ApplicationRoot &a )
{
	IECore::ConstObjectPtr o = a.getClipboardContents();
	if( o )
	{
		return o->copy();
	}
	return nullptr;
}

struct ClipboardSlotCaller
{
	void operator()( boost::python::object slot, ApplicationRootPtr a )
	{
		try
		{
			slot( a );
		}
		catch( const error_already_set &e )
		{
			IECorePython::ExceptionAlgo::translatePythonException();
		}
	}
};

} // namespace

void GafferModule::bindApplicationRoot()
{
	{
		scope s = IECorePython::RunTimeTypedClass<ApplicationRoot, ApplicationRootWrapper>()
			.def( init<>() )
			.def( init<const std::string &>() )
			.def( "getClipboardContents", &getClipboardContents )
			.def( "setClipboardContents", &ApplicationRoot::setClipboardContents )
			.def( "clipboardContentsChangedSignal", &ApplicationRoot::clipboardContentsChangedSignal, return_internal_reference<1>() )
			.def( "savePreferences", (void (ApplicationRoot::*)()const)&ApplicationRoot::savePreferences )
			.def( "savePreferences", (void (ApplicationRoot::*)( const std::filesystem::path & )const)&ApplicationRoot::savePreferences )
			.def( "preferencesLocation", &ApplicationRoot::preferencesLocation )
		;

		SignalClass<ApplicationRoot::ClipboardSignal, DefaultSignalCaller<ApplicationRoot::ClipboardSignal>, ClipboardSlotCaller>( "ClipboardSignal" );
	}

	GafferBindings::NodeClass<Preferences>();

}
