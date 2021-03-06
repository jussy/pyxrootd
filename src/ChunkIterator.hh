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

#ifndef CHUNKITERATOR_HH_
#define CHUNKITERATOR_HH_

#include "PyXRootD.hh"
#include "PyXRootDFile.hh"

namespace PyXRootD
{
  //----------------------------------------------------------------------------
  //! Iterator class for looping over a file and yielding chunks from it
  //----------------------------------------------------------------------------
  class ChunkIterator
  {
    public:
      PyObject_HEAD
      File    *file;
      uint64_t blocksize;
      uint32_t startOffset;
      uint32_t currentOffset;
  };

  //----------------------------------------------------------------------------
  //! __init__
  //----------------------------------------------------------------------------
  static int ChunkIterator_init(ChunkIterator *self, PyObject *args)
  {
    if ( !PyArg_ParseTuple( args, "OIk", &self->file,
        &self->blocksize, &self->startOffset ) ) return -1;

    self->currentOffset = self->startOffset;
    return 0;
  }

  //----------------------------------------------------------------------------
  //! __iter__
  //----------------------------------------------------------------------------
  static PyObject* ChunkIterator_iter(ChunkIterator *self)
  {
    Py_INCREF(self);
    return (PyObject*) self;
  }

  //----------------------------------------------------------------------------
  //! __iternext__
  //!
  //! Dumb implementation, should use prefetching/readv
  //----------------------------------------------------------------------------
  static PyObject* ChunkIterator_iternext(ChunkIterator *self)
  {
    PyObject *line = self->file->Read( self->file,
        Py_BuildValue( "kI", self->currentOffset, self->blocksize ), NULL );

    if ( !PyString_Size( PyTuple_GetItem( line, 1 ) ) ) {
      //------------------------------------------------------------------------
      // Raise standard StopIteration exception with empty value
      //------------------------------------------------------------------------
      PyErr_SetNone( PyExc_StopIteration );
      return NULL;
    }

    self->currentOffset += self->blocksize;
    return line;
  }

  //----------------------------------------------------------------------------
  //! ChunkIterator type structure
  //----------------------------------------------------------------------------
  static PyTypeObject ChunkIteratorType = {
      PyObject_HEAD_INIT(NULL)
      0,                                          /* ob_size */
      "client.File.ChunkIterator",                /* tp_name */
      sizeof(ChunkIterator),                      /* tp_basicsize */
      0,                                          /* tp_itemsize */
      0,                                          /* tp_dealloc */
      0,                                          /* tp_print */
      0,                                          /* tp_getattr */
      0,                                          /* tp_setattr */
      0,                                          /* tp_compare */
      0,                                          /* tp_repr */
      0,                                          /* tp_as_number */
      0,                                          /* tp_as_sequence */
      0,                                          /* tp_as_mapping */
      0,                                          /* tp_hash */
      0,                                          /* tp_call */
      0,                                          /* tp_str */
      0,                                          /* tp_getattro */
      0,                                          /* tp_setattro */
      0,                                          /* tp_as_buffer */
      Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,  /* tp_flags */
      "Internal chunk iterator object",           /* tp_doc */
      0,                                          /* tp_traverse */
      0,                                          /* tp_clear */
      0,                                          /* tp_richcompare */
      0,                                          /* tp_weaklistoffset */
      (getiterfunc)  ChunkIterator_iter,          /* tp_iter */
      (iternextfunc) ChunkIterator_iternext,      /* tp_iternext */
      0,                                          /* tp_methods */
      0,                                          /* tp_members */
      0,                                          /* tp_getset */
      0,                                          /* tp_base */
      0,                                          /* tp_dict */
      0,                                          /* tp_descr_get */
      0,                                          /* tp_descr_set */
      0,                                          /* tp_dictoffset */
      (initproc) ChunkIterator_init,              /* tp_init */
  };
}

#endif /* CHUNKITERATOR_HH_ */
