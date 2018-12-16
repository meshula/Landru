#pragma once

#ifdef _MSC_VER
# define itoa _itoa
#endif


class BsonCompiler {
public:
	// temp for bson parsing
	std::vector<int> bsonArrayNesting;
	int bsonCurrArrayIndex;

	BsonCompiler() : bsonCurrArrayIndex(0) {}

	std::shared_ptr<Lab::Bson> compileBson(Landru::ASTNode* rootNode) 
	{
		bsonArrayNesting.clear();
		bsonCurrArrayIndex = 0;
		std::shared_ptr<Lab::Bson> b = std::make_shared<Lab::Bson>();
		compileBsonData(rootNode, b->b);
		bson_finish(b->b);
		bson_print(b->b);
		return b;
	}

	void compileBsonData(Landru::ASTNode* rootNode, bson* b)
	{
		using namespace Landru;
		for (ASTConstIter i = rootNode->children.begin(); i != rootNode->children.end(); ++i)
			compileBsonDataElement(*i, b);
	}

	void compileBsonDataElement(Landru::ASTNode* rootNode, bson* b)
	{
		using namespace Landru;
		switch (rootNode->token) {

		case kTokenDataObject:
		{
			ASSEMBLER_TRACE(kTokenDataObject);
			if (bsonArrayNesting.size() > 0) {
				char indexStr[16];
				itoa(bsonCurrArrayIndex, indexStr, 10);
				//printf("----> %s\n", indexStr);
				bson_append_start_object(b, indexStr);
				compileBsonData(rootNode, b); // recurse
				bson_append_finish_object(b);
				++bsonCurrArrayIndex;
			}
			else
				compileBsonData(rootNode, b); // recurse
			break;
		}

		case kTokenDataArray:
		{
			ASSEMBLER_TRACE(kTokenDataArray);
			bsonArrayNesting.push_back(bsonCurrArrayIndex);
			bsonCurrArrayIndex = 0;
			compileBsonData(rootNode, b); // recurse
			bsonCurrArrayIndex = bsonArrayNesting.back();
			bsonArrayNesting.pop_back();
			break;
		}

		case kTokenDataElement:
		{
			ASSEMBLER_TRACE(kTokenDataElement);
			ASTConstIter i = rootNode->children.begin();
			if ((*i)->token == kTokenDataIntLiteral) {
				int val = atoi((*i)->str2.c_str());
				bson_append_int(b, rootNode->str2.c_str(), val);
			}
			else if ((*i)->token == kTokenDataFloatLiteral)
			{
				float val = (float)atof(rootNode->str2.c_str());
				bson_append_double(b, rootNode->str2.c_str(), val);
			}
			else if ((*i)->token == kTokenDataNullLiteral) {
				bson_append_null(b, rootNode->str2.c_str());
			}
			else {
				bson_append_start_object(b, rootNode->str2.c_str());
				compileBsonData(rootNode, b); // recurse
				bson_append_finish_object(b);
			}
			break;
		}

		case kTokenDataFloatLiteral:
		{
			ASSEMBLER_TRACE(kTokenDataFloatLiteral);
			float val = (float)atof(rootNode->str2.c_str());
			bson_append_double(b, rootNode->str2.c_str(), val);
			break;
		}

		case kTokenDataIntLiteral:
		{
			ASSEMBLER_TRACE(kTokenDataIntLiteral);
			int val = atoi(rootNode->str2.c_str());
			bson_append_int(b, rootNode->str2.c_str(), val);
			break;
		}

		case kTokenDataNullLiteral:
		{
			ASSEMBLER_TRACE(kTokenDataNullLiteral);
			bson_append_null(b, rootNode->str2.c_str());
			break;
		}

		case kTokenDataStringLiteral:
		{
			ASSEMBLER_TRACE(kTokenDataStringLiteral);
			bson_append_string(b, rootNode->str2.c_str(), rootNode->str1.c_str());
			break;
		}

		default:
			break;
		}
	}

};


