#ifndef FONTIO_H
#define	FONTIO_H

#include <boost/crc.hpp>

class FontIO
{
    std::string crc(const std::string& file)
    {
        boost::crc_32_type crc32;
        std::ifstream  ifs( file, std::ios_base::binary );

        if ( ifs )
        {
            do
            {
                char  buffer[ 1024 ];

                ifs.read( buffer, 1024 );
                crc32.process_bytes( buffer, ifs.gcount() );
            } while ( ifs );
        }
        else
        {
            // crap
        }
        return crc32.checksum();
    }
};

#endif	/* FONTIO_H */

