// record_store_v1_capped.h

/**
*    Copyright (C) 2013 10gen Inc.
*
*    This program is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*    As a special exception, the copyright holders give permission to link the
*    code of portions of this program with the OpenSSL library under certain
*    conditions as described in each individual source file and distribute
*    linked combinations including the program with the OpenSSL library. You
*    must comply with the GNU Affero General Public License in all respects for
*    all of the code used other than as permitted herein. If you modify file(s)
*    with this exception, you may extend this exception to your version of the
*    file(s), but you are not obligated to do so. If you do not wish to do so,
*    delete this exception statement from your version. If you delete this
*    exception statement from all source files in the program, then also delete
*    it in the license file.
*/

#pragma once

#include "mongo/base/owned_pointer_vector.h"
#include "mongo/db/diskloc.h"
#include "mongo/db/structure/record_store_v1_base.h"

namespace mongo {

    class Collection;
    class MAdvise;

    class CappedRecordStoreV1 : public RecordStoreV1Base {
    public:
        CappedRecordStoreV1( Collection* collection,
                             const StringData& ns,
                             NamespaceDetails* details,
                             ExtentManager* em,
                             bool isSystemIndexes );

        virtual ~CappedRecordStoreV1();

        const char* name() const { return "CappedRecordStoreV1"; }

        virtual Status truncate();

        /**
         * Truncate documents newer than the document at 'end' from the capped
         * collection.  The collection cannot be completely emptied using this
         * function.  An assertion will be thrown if that is attempted.
         * @param inclusive - Truncate 'end' as well iff true
         * XXX: this will go away soon, just needed to move for now
         */
        void temp_cappedTruncateAfter( DiskLoc end, bool inclusive );

        virtual RecordIterator* getIterator( const DiskLoc& start, bool tailable,
                                             const CollectionScanParams::Direction& dir) const;

        virtual bool compactSupported() const { return false; }

        virtual Status compact( RecordStoreCompactAdaptor* adaptor,
                                const CompactOptions* options,
                                CompactStats* stats );

        // Start from firstExtent by default.
        DiskLoc firstRecord( const DiskLoc &startExtent = DiskLoc() ) const;
        // Start from lastExtent by default.
        DiskLoc lastRecord( const DiskLoc &startExtent = DiskLoc() ) const;

    protected:

        virtual bool isCapped() const { return true; }

        virtual StatusWith<DiskLoc> allocRecord( int lengthWithHeaders, int quotaMax );

        virtual void addDeletedRec(const DiskLoc& dloc);

    private:
        // -- start copy from cap.cpp --
        void compact();
        DiskLoc& cappedFirstDeletedInCurExtent();
        void cappedCheckMigrate();
        DiskLoc __capAlloc( int len );
        bool inCapExtent( const DiskLoc &dl ) const;
        DiskLoc& cappedListOfAllDeletedRecords();
        DiskLoc& cappedLastDelRecLastExtent();
        bool capLooped() const;
        Extent *theCapExtent() const;
        bool nextIsInCapExtent( const DiskLoc &dl ) const;
        void advanceCapExtent( const StringData& ns );
        void cappedTruncateLastDelUpdate();

        /**
         * Truncate documents newer than the document at 'end' from the capped
         * collection.  The collection cannot be completely emptied using this
         * function.  An assertion will be thrown if that is attempted.
         * @param inclusive - Truncate 'end' as well iff true
         */
        void cappedTruncateAfter(const char *ns, DiskLoc end, bool inclusive);

        void _maybeComplain( int len ) const;

        // -- end copy from cap.cpp --

        Collection* _collection;

        OwnedPointerVector<MAdvise> _extentAdvice;

    };


}
