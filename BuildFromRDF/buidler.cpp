#include "TripleBitBuilder.h"
#include "OSFile.h"
#include <raptor.h>

#include <string>
#include <set>

using namespace std;

static TempFile rawFacts("./test");

void rdfParser(void* user_data, const raptor_statement* triple)
{
	TripleBitBuilder *builder = (TripleBitBuilder*)user_data;

	char* predicate = (char*)triple->predicate;
	char* subject = (char*)triple->subject;
	char* object = (char*)triple->object;


	if(strlen(predicate) && strlen(subject) && strlen(object))
		builder-> NTriplesParse(subject, predicate, object, rawFacts);
}

void parserRDFFile(string fileName, raptor_statement_handler handler, void* user_data)
{
	raptor_parser *rdf_parser;
	raptor_uri *uri, *base_uri;
	unsigned char* uri_string;

	raptor_init();
	rdf_parser = raptor_new_parser("rdfxml");
	raptor_set_statement_handler(rdf_parser, user_data, handler);

	uri_string = raptor_uri_filename_to_uri_string(fileName.c_str());
	uri = raptor_new_uri(uri_string);
	base_uri = raptor_uri_copy(uri);
	raptor_parse_file(rdf_parser, uri, base_uri);

	raptor_free_parser(rdf_parser);
	raptor_free_uri(base_uri);
	raptor_free_uri(uri);
	raptor_free_memory(uri_string);

	raptor_finish();
}

//char* DATABASE_PATH;
int main(int argc, char* argv[])
{
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <RDF file name> <Database Directory>\n", argv[0]);
		return -1;
	}

	if(OSFile::DirectoryExists(argv[1]) == false) {
		OSFile::MkDir(argv[1]);
	}

	TripleBitBuilder* builder = new TripleBitBuilder(argv[2]);
	parserRDFFile(argv[1], rdfParser, builder);

	TempFile facts(argv[1]);
	builder->resolveTriples(rawFacts, facts);
	facts.discard();

	builder->endBuild();
	delete builder;

	return 0;
}