//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <earthwings@gentoo.org>
//

#ifndef MARBLE_KMLSTYLEMAPTAGWRITER_H
#define MARBLE_KMLSTYLEMAPTAGWRITER_H

#include "GeoTagWriter.h"

namespace Marble
{

class KmlStyleMapTagWriter : public GeoTagWriter
{
public:
    virtual bool write( const GeoDataObject &node, GeoWriter& writer ) const;
};

}

#endif