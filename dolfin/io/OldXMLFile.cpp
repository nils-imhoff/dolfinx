// Copyright (C) 2009 Ola Skavhaug
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// Modified by Garth N. Wells, 2009.
//
// First added:  2009-03-03
// Last changed: 2011-03-31

#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>

#include <dolfin/common/types.h>
#include <dolfin/common/constants.h>
#include <dolfin/la/GenericMatrix.h>
#include <dolfin/log/log.h>
#include <dolfin/mesh/LocalMeshData.h>
#include <dolfin/mesh/Mesh.h>
#include <dolfin/mesh/MeshEntity.h>
#include <dolfin/mesh/MeshFunction.h>
#include <dolfin/plot/FunctionPlotData.h>
#include "XMLArray.h"
#include "XMLFunctionPlotData.h"
#include "XMLLocalMeshDataDistributed.h"
#include "XMLMesh.h"
#include "XMLMeshFunction.h"
#include "XMLParameters.h"
#include "XMLVector.h"
#include "OldXMLFile.h"

using namespace dolfin;

//-----------------------------------------------------------------------------
OldXMLFile::OldXMLFile(const std::string filename) : GenericFile(filename), sax(0),
                                               outstream(0)
{
  // Set up the output stream (to file)
  outstream = new std::ofstream();

  // Set up the sax handler.
  sax = new xmlSAXHandler();

  // Set up handlers for parser events
  sax->startDocument = sax_start_document;
  sax->endDocument   = sax_end_document;
  sax->startElement  = sax_start_element;
  sax->endElement    = sax_end_element;
  sax->warning       = sax_warning;
  sax->error         = sax_error;
  sax->fatalError    = sax_fatal_error;
}
//-----------------------------------------------------------------------------
OldXMLFile::OldXMLFile(std::ostream& s) : GenericFile(""), sax(0), outstream(&s)
{
  // Set up the sax handler.
  sax = new xmlSAXHandler();

  // Set up handlers for parser events
  sax->startDocument = sax_start_document;
  sax->endDocument   = sax_end_document;
  sax->startElement  = sax_start_element;
  sax->endElement    = sax_end_element;
  sax->warning       = sax_warning;
  sax->error         = sax_error;
  sax->fatalError    = sax_fatal_error;
}
//-----------------------------------------------------------------------------
OldXMLFile::~OldXMLFile()
{
  delete sax;

  // Only delete outstream if it is an 'ofstream'
  std::ofstream* outfile = dynamic_cast<std::ofstream*>(outstream);
  if (outfile)
  {
    outfile = 0;
    delete outstream;
  }
}
//-----------------------------------------------------------------------------
void OldXMLFile::parse()
{
  // Parse file
  xmlSAXUserParseFile(sax, (void *) this, filename.c_str());
}
//-----------------------------------------------------------------------------
void OldXMLFile::push(XMLHandler* handler)
{
  handlers.push(handler);
}
//-----------------------------------------------------------------------------
void OldXMLFile::pop()
{
  assert(!handlers.empty());
  handlers.pop();
}
//-----------------------------------------------------------------------------
XMLHandler* OldXMLFile:: top()
{
  assert(!handlers.empty());
  return handlers.top();
}
//-----------------------------------------------------------------------------
void OldXMLFile::start_element(const xmlChar *name, const xmlChar **attrs)
{
  handlers.top()->start_element(name, attrs);
}
//-----------------------------------------------------------------------------
void OldXMLFile::end_element(const xmlChar *name)
{
  handlers.top()->end_element(name);
}
//-----------------------------------------------------------------------------
void OldXMLFile::open_file()
{
  // Convert to ofstream
  std::ofstream* outfile = dynamic_cast<std::ofstream*>(outstream);
  if (outfile)
  {
    // Open file
    outfile->open(filename.c_str());

    // Go to end of file
    outfile->seekp(0, std::ios::end);
  }
  XMLDolfin::write_start(*outstream);
}
//-----------------------------------------------------------------------------
void OldXMLFile::close_file()
{
  XMLDolfin::write_end(*outstream);

  // Get file path and extension
  const boost::filesystem::path path(filename);
  const std::string extension = boost::filesystem::extension(path);
  if (extension == ".gz")
  {
    error("Compressed XML output not yet supported.");
  }
  else
  {
    // Convert to ofstream
    std::ofstream* outfile = dynamic_cast<std::ofstream*>(outstream);
    if (outfile)
      outfile->close();
  }
}
//-----------------------------------------------------------------------------
// Callback functions for the SAX interface
//-----------------------------------------------------------------------------
void dolfin::sax_start_document(void *ctx)
{
  // Do nothing
}
//-----------------------------------------------------------------------------
void dolfin::sax_end_document(void *ctx)
{
  // Do nothing
}
//-----------------------------------------------------------------------------
void dolfin::sax_start_element(void *ctx, const xmlChar *name,
                               const xmlChar **attrs)
{
  ( (OldXMLFile*) ctx )->start_element(name, attrs);
}
//-----------------------------------------------------------------------------
void dolfin::sax_end_element(void *ctx, const xmlChar *name)
{
  ( (OldXMLFile*) ctx )->end_element(name);
}
//-----------------------------------------------------------------------------
void dolfin::sax_warning(void *ctx, const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  char buffer[DOLFIN_LINELENGTH];
  vsnprintf(buffer, DOLFIN_LINELENGTH, msg, args);
  warning("Incomplete XML data: " + std::string(buffer));
  va_end(args);
}
//-----------------------------------------------------------------------------
void dolfin::sax_error(void *ctx, const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  char buffer[DOLFIN_LINELENGTH];
  vsnprintf(buffer, DOLFIN_LINELENGTH, msg, args);
  error("Illegal XML data: " + std::string(buffer));
  va_end(args);
}
//-----------------------------------------------------------------------------
void dolfin::sax_fatal_error(void *ctx, const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  char buffer[DOLFIN_LINELENGTH];
  vsnprintf(buffer, DOLFIN_LINELENGTH, msg, args);
  error("Illegal XML data: " + std::string(buffer));
  va_end(args);
}
//-----------------------------------------------------------------------------
void dolfin::rng_parser_error(void *user_data, xmlErrorPtr error)
{
  char *file = error->file;
  char *message = error->message;
  int line = error->line;
  xmlNodePtr node;
  node = (xmlNode*)error->node;
  std::string buffer;
  buffer = message;
  int length = buffer.length();
  buffer.erase(length-1);
  if (node != NULL)
  {
    warning("%s:%d: element %s: Relax-NG parser error: %s",
            file, line, node->name, buffer.c_str());
  }
}
//-----------------------------------------------------------------------------
void dolfin::rng_valid_error(void *user_data, xmlErrorPtr error)
{
  char *file = error->file;
  char *message = error->message;
  int line = error->line;
  xmlNodePtr node;
  node = (xmlNode*)error->node;
  std::string buffer;
  buffer = message;
  int length = buffer.length();
  buffer.erase(length-1);
  warning("%s:%d: element %s: Relax-NG validity error: %s",
          file, line, node->name, buffer.c_str());
}
//-----------------------------------------------------------------------------
