//---------------------------------------------------------------------------
// csved_exclude.cpp
//
// exclude fields from output
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_exclude.h"
#include "csved_strings.h"
#include "csved_evalvars.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register exclude command
//---------------------------------------------------------------------------

static RegisterCommand <ExcludeCommand> rc1_(
	CMD_EXCLUDE,
	"exclude fields from output"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const EXCL_HELP = {
	"exclude specific CSV input fields from output\n"
	"usage: csvfix exclude  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tlist of fields to exclude\n"
	"  -if expr\texclude fields specified by -f if expr evaluates to true\n"
	"#ALL"
};

//----------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

ExcludeCommand ::ExcludeCommand( const string & name,
								const string & desc )
		: Command( name, desc, EXCL_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_IF, false, 1 ) );
}

//---------------------------------------------------------------------------
// Get user specifie doptions and then read input, removing excluded cols.
// Now does exclude only if expression specified by -if evaluates true.
//---------------------------------------------------------------------------

int ExcludeCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( EvalExprOnRow( io, row ) ) {
			Exclude( row );
		}
		io.WriteRow( row );
	}

	return 0;
}


//----------------------------------------------------------------------------
// See if row should be excluded based on expression
//----------------------------------------------------------------------------

bool ExcludeCommand :: EvalExprOnRow( IOManager & io, const CSVRow & row ) {

	if ( mExpr.IsCompiled() ) {
		AddVars( mExpr, io, row );
		string s = mExpr.Evaluate();
		return ALib::Expression::ToBool( s );
	}
	else {
		return true;
	}
}

//---------------------------------------------------------------------------
// Handle all user options with error checking. Currently possible to pass
// empty expression, but probably shouldn't be.
//---------------------------------------------------------------------------

void ExcludeCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	string es = cmd.GetValue( FLAG_IF, "" );
	if ( es != "" ) {
		mExpr.Compile( es );
	}
	string sn = cmd.GetValue( FLAG_COLS, ""  );
	CommaListToIndex( ALib::CommaList( sn ), mFields );
	if ( mFields.size() == 0 ) {
		CSVTHROW( "Field list  specified by " << FLAG_COLS
				<< " cannot be empty" );
	}
}

//---------------------------------------------------------------------------
// Copy all fields that are not excluded to new row. Change parameter
// to have new row contents.
//---------------------------------------------------------------------------

void ExcludeCommand :: Exclude(  CSVRow & r ) const {

	CSVRow out;

	for ( unsigned int i = 0; i < r.size(); i++ ) {
		if ( ! ALib::Contains( mFields, i ) ) {
			out.push_back( r.at( i ) );
		}
	}
	r.swap( out );
}


} // end namespace

// end

