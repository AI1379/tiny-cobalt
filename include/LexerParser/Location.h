//
// Created by Renatus Madrigal on 01/18/2025
//

#ifndef TINY_COBALT_INCLUDE_LEXERPARSER_LOCATION_H_
#define TINY_COBALT_INCLUDE_LEXERPARSER_LOCATION_H_

#include <string>
#include <iostream>

namespace TinyCobalt::LexerParser {
    // Adopted from bison generated location.hh
    
    /// A point in a source file.
    class Position {
    public:
        /// Type for file name.
        typedef const std::string filename_type;
        /// Type for line and column numbers.
        typedef int counter_type;

        /// Construct a position.
        explicit Position(filename_type *f = nullptr, counter_type l = 1, counter_type c = 1) :
            filename(f), line(l), column(c) {}


        /// Initialization.
        void initialize(filename_type *fn = nullptr, counter_type l = 1, counter_type c = 1) {
            filename = fn;
            line = l;
            column = c;
        }

        /** \name Line and Column related manipulators
         ** \{ */
        /// (line related) Advance to the COUNT next lines.
        void lines(counter_type count = 1) {
            if (count) {
                column = 1;
                line = add_(line, count, 1);
            }
        }

        /// (column related) Advance to the COUNT next columns.
        void columns(counter_type count = 1) { column = add_(column, count, 1); }
        /** \} */

        /// File name to which this position refers.
        filename_type *filename;
        /// Current line number.
        counter_type line;
        /// Current column number.
        counter_type column;

    private:
        /// Compute max (min, lhs+rhs).
        static counter_type add_(counter_type lhs, counter_type rhs, counter_type min) {
            return lhs + rhs < min ? min : lhs + rhs;
        }
    };

    /// Add \a width columns, in place.
    inline Position &operator+=(Position &res, Position::counter_type width) {
        res.columns(width);
        return res;
    }

    /// Add \a width columns.
    inline Position operator+(Position res, Position::counter_type width) { return res += width; }

    /// Subtract \a width columns, in place.
    inline Position &operator-=(Position &res, Position::counter_type width) { return res += -width; }

    /// Subtract \a width columns.
    inline Position operator-(Position res, Position::counter_type width) { return res -= width; }

    /** \brief Intercept output stream redirection.
     ** \param ostr the destination output stream
     ** \param pos a reference to the position to redirect
     */
    template<typename Char>
    std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &ostr, const Position &pos) {
        if (pos.filename)
            ostr << *pos.filename << ':';
        return ostr << pos.line << '.' << pos.column;
    }

    /// Two points in a source file.
    class Location {
    public:
        /// Type for file name.
        typedef Position::filename_type filename_type;
        /// Type for line and column numbers.
        typedef Position::counter_type counter_type;

        /// Construct a location from \a b to \a e.
        Location(const Position &b, const Position &e) : begin(b), end(e) {}

        /// Construct a 0-width location in \a p.
        explicit Location(const Position &p = Position()) : begin(p), end(p) {}

        /// Construct a 0-width location in \a f, \a l, \a c.
        explicit Location(filename_type *f, counter_type l = 1, counter_type c = 1) : begin(f, l, c), end(f, l, c) {}


        /// Initialization.
        void initialize(filename_type *f = nullptr, counter_type l = 1, counter_type c = 1) {
            begin.initialize(f, l, c);
            end = begin;
        }

        /** \name Line and Column related manipulators
         ** \{ */
    public:
        /// Reset initial location to final location.
        void step() { begin = end; }

        /// Extend the current location to the COUNT next columns.
        void columns(counter_type count = 1) { end += count; }

        /// Extend the current location to the COUNT next lines.
        void lines(counter_type count = 1) { end.lines(count); }
        /** \} */


    public:
        /// Beginning of the located region.
        Position begin;
        /// End of the located region.
        Position end;
    };

    /// Join two locations, in place.
    inline Location &operator+=(Location &res, const Location &end) {
        res.end = end.end;
        return res;
    }

    /// Join two locations.
    inline Location operator+(Location res, const Location &end) { return res += end; }

    /// Add \a width columns to the end position, in place.
    inline Location &operator+=(Location &res, Location::counter_type width) {
        res.columns(width);
        return res;
    }

    /// Add \a width columns to the end position.
    inline Location operator+(Location res, Location::counter_type width) { return res += width; }

    /// Subtract \a width columns to the end position, in place.
    inline Location &operator-=(Location &res, Location::counter_type width) { return res += -width; }

    /// Subtract \a width columns to the end position.
    inline Location operator-(Location res, Location::counter_type width) { return res -= width; }

    /** \brief Intercept output stream redirection.
     ** \param ostr the destination output stream
     ** \param loc a reference to the location to redirect
     **
     ** Avoid duplicate information.
     */
    template<typename Char>
    std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &ostr, const Location &loc) {
        Location::counter_type end_col = 0 < loc.end.column ? loc.end.column - 1 : 0;
        ostr << loc.begin;
        if (loc.end.filename && (!loc.begin.filename || *loc.begin.filename != *loc.end.filename))
            ostr << '-' << loc.end.filename << ':' << loc.end.line << '.' << end_col;
        else if (loc.begin.line < loc.end.line)
            ostr << '-' << loc.end.line << '.' << end_col;
        else if (loc.begin.column < end_col)
            ostr << '-' << end_col;
        return ostr;
    }
} // namespace TinyCobalt::LexerParser

#endif // TINY_COBALT_INCLUDE_LEXERPARSER_LOCATION_H_