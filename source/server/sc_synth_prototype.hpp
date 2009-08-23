//  prototype of a supercollider-synthdef-based synth prototype
//  Copyright (C) 2009 Tim Blechmann
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.

#ifndef SC_SYNTH_PROTOTYPE_HPP
#define SC_SYNTH_PROTOTYPE_HPP

#include <boost/filesystem/path.hpp>

#include "synth_prototype.hpp"
#include "sc_synthdef.hpp"

namespace nova
{

using boost::filesystem::path;

/* read synthdefs from path pattern */
void sc_read_synthdefs_dir(class synth_factory & factory, path const & dir);
void sc_read_synthdef(class synth_factory & factory, path const & filename);
void sc_read_synthdef(class synth_factory & factory, void * buffer, std::size_t size);

class sc_synth_prototype:
    public synth_prototype
{
public:
    sc_synth_prototype(sc_synthdef const & sd):
        synth_prototype(sd.name()), synthdef(sd)
    {}

private:
    virtual abstract_synth * create_instance(int);

    sc_synthdef synthdef;
};

typedef boost::intrusive_ptr<sc_synth_prototype> sc_synth_prototype_ptr;



} /* namespace nova */


#endif /* SC_SYNTH_PROTOTYPE_HPP */
