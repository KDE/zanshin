/*
  This file is part of libkdepim.

  Copyright (c) 2006 Christian Schaarschmidt <schaarsc@gmx.de>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kmailcompletion.h"
#include <KDebug>
#include <QRegExp>

using namespace KPIM;

KMailCompletion::KMailCompletion()
{
    setIgnoreCase( true );
}

void KMailCompletion::clear()
{
    m_keyMap.clear();
    KCompletion::clear();
}

QString KMailCompletion::makeCompletion( const QString &string )
{
    QString match = KCompletion::makeCompletion( string );

    // this should be in postProcessMatch, but postProcessMatch is const and will not allow nextMatch
    if ( !match.isEmpty() ){
        const QString firstMatch( match );
        while ( match.indexOf( QRegExp( QLatin1String("(@)|(<.*>)") ) ) == -1 ) {
            /* local email do not require @domain part, if match is an address we'll
       * find last+first <match> in m_keyMap and we'll know that match is
       * already a valid email.
       *
       * Distribution list do not have last+first <match> entry, they will be
       * in mailAddr
       */
            const QStringList &mailAddr = m_keyMap[ match ]; //get all mailAddr for this keyword
            bool isEmail = false;
            for ( QStringList::ConstIterator sit( mailAddr.begin() ), sEnd( mailAddr.end() );
                  sit != sEnd; ++sit ) {
                if ( (*sit).indexOf( QLatin1Char('<') + match + QLatin1Char('>') ) != -1 || (*sit) == match ) {
                    isEmail = true;
                    break;
                }
            }

            if ( !isEmail ) {
                // match is a keyword, skip it and try to find match <email@domain>
                match = nextMatch();
                if ( firstMatch == match ){
                    match.clear();
                    break;
                }
            } else {
                break;
            }
        }
    }
    return match;
}

void KMailCompletion::addItemWithKeys( const QString &email, int weight,
                                       const QStringList *keyWords )
{
    Q_ASSERT( keyWords != 0 );
    QStringList::ConstIterator end =  keyWords->constEnd();
    for ( QStringList::ConstIterator it( keyWords->constBegin() ); it != end; ++it ) {
        QStringList &emailList = m_keyMap[ (*it) ]; //lookup email-list for given keyword
        if ( emailList.indexOf( email ) == -1 ) {   //add email if not there
            emailList.append( email );
        }
        addItem( (*it), weight );                   //inform KCompletion about keyword
    }
}

void KMailCompletion::postProcessMatches( QStringList *pMatches ) const
{
    Q_ASSERT( pMatches != 0 );
    if ( pMatches->isEmpty() ) {
        return;
    }

    //KCompletion has found the keywords for us, we can now map them to mail-addr
    QSet< QString > mailAddrDistinct;
    for ( QStringList::ConstIterator sit( pMatches->begin() ), sEnd( pMatches->end() );
          sit != sEnd; ++sit ) {
        const QStringList &mailAddr = m_keyMap[ (*sit) ]; //get all mailAddr for this keyword
        for ( QStringList::ConstIterator sit( mailAddr.begin() ), sEnd( mailAddr.end() );
              sit != sEnd; ++sit ) {
            mailAddrDistinct.insert( *sit );  //store mailAddr, QSet will make them unique
        }
    }
    pMatches->clear();                        //delete keywords
    (*pMatches) += mailAddrDistinct.toList(); //add emailAddr
}
