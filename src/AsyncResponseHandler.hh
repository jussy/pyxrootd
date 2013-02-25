//------------------------------------------------------------------------------
// Copyright (c) 2012-2013 by European Organization for Nuclear Research (CERN)
// Author: Justin Salmon <jsalmon@cern.ch>
//------------------------------------------------------------------------------
// XRootD is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// XRootD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with XRootD.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#ifndef ASYNCRESPONSEHANDLER_HH_
#define ASYNCRESPONSEHANDLER_HH_

#include <Python.h>

#include "XrdCl/XrdClXRootDResponses.hh"

#include "StatInfoType.hh"

namespace XrdClBind
{
  //----------------------------------------------------------------------------
  //! Generic asynchronous response handler
  //----------------------------------------------------------------------------
  template<class Type>
  class AsyncResponseHandler: public XrdCl::ResponseHandler
  {
    public:
      //------------------------------------------------------------------------
      //! Constructor
      //------------------------------------------------------------------------
      AsyncResponseHandler(Type *responseObject, PyTypeObject *bindType,
          PyObject *callback)
      {
        this->responseObject = responseObject;
        this->bindType = bindType;
        this->callback = callback;
      }

      //------------------------------------------------------------------------
      //! Handle the asynchronous response call
      //------------------------------------------------------------------------
      void HandleResponse(XrdCl::XRootDStatus *status,
          XrdCl::AnyObject *response)
      {
        //----------------------------------------------------------------------
        // Ensure we hold the Global Interpreter Lock
        //----------------------------------------------------------------------
        PyGILState_STATE state = PyGILState_Ensure();

        //----------------------------------------------------------------------
        // Convert the XRootDStatus object
        //----------------------------------------------------------------------
        PyObject *statusArgs = Py_BuildValue("(HHIs)", status->status,
            status->code, status->errNo, status->GetErrorMessage().c_str());
        if (!statusArgs) return;

        PyObject *statusBind = PyObject_CallObject(
            (PyObject *) &XRootDStatusType, statusArgs);
        if (!statusBind) return;
        Py_DECREF(statusArgs);

        //----------------------------------------------------------------------
        // Convert the response object, if any
        //----------------------------------------------------------------------
        PyObject *responseBind = ParseResponse(response);
        if (!responseBind) return;

        //----------------------------------------------------------------------
        // Build the callback arguments
        //----------------------------------------------------------------------
        PyObject *args = Py_BuildValue("(OO)", statusBind, responseBind);
        if (!args) return;

        //----------------------------------------------------------------------
        // Invoke the Python callback
        //----------------------------------------------------------------------
        PyObject *result = PyObject_CallObject(this->callback, args);
        if (!result) return;
        Py_DECREF(args);

        //----------------------------------------------------------------------
        // Clean up
        //----------------------------------------------------------------------
        Py_DECREF(statusBind);
        Py_DECREF(responseBind);
        Py_DECREF(result);
        Py_DECREF(this->callback);
        // Release the GIL
        PyGILState_Release(state);

        delete status;
        delete response;
        // Commit suicide ...
        delete this;
      }

      //------------------------------------------------------------------------
      //! Parse out and convert the AnyObject response to a mapping type
      //------------------------------------------------------------------------
      PyObject* ParseResponse(XrdCl::AnyObject *response)
      {
        PyObject *response_bind;
        response->Get(this->responseObject);

        //----------------------------------------------------------------------
        // Build the arguments for creating the response mapping type. We cast
        // the response object to a void * before packing it into a PyCObject.
        //
        // The CObject API is deprecated as of Python 2.7
        //----------------------------------------------------------------------
        PyObject *response_args = Py_BuildValue("(O)",
            PyCObject_FromVoidPtr((void *) this->responseObject, NULL));
        if (!response_args) {
          return NULL;
        }

        //----------------------------------------------------------------------
        // Call the constructor of the bound type.
        //----------------------------------------------------------------------
        response_bind = PyObject_CallObject((PyObject *) this->bindType,
            response_args);
        if (!response_bind) {
          return NULL;
        }

        return response_bind;
      }

    private:
      Type *responseObject;
      PyTypeObject *bindType;
      PyObject *callback;
  };
}

#endif /* ASYNCRESPONSEHANDLER_HH_ */