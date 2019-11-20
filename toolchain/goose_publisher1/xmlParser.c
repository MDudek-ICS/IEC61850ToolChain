/**
 * section: Tree
 * synopsis: Navigates a tree to print element names
 * purpose: Parse a file to a tree, use xmlDocGetRootElement() to
 *          get the root element, then walk the document and print
 *          all the element name in document order.
 * usage: tree1 filename_or_URL
 * test: tree1 test2.xml > tree1.tmp && diff tree1.tmp $(srcdir)/tree1.res
 * author: Dodji Seketeli
 * copy: see Copyright for the status of this software.
 */
#include <stdio.h>
#include "xmlParser.h"
#include <unistd.h>
#define ATTACKSCENARIOXML "/home/liyuan/IEC61850ToolChain/toolchain/goose_publisher1/AttackScenarioConfiguration.xml"
/*
 *To compile this file using gcc you can type
 *gcc `xml2-config --cflags --libs` -o xmlexample libxml2-example.c
 * gcc -I/usr/include/libxml2  xmlParser.c -o xmlParser -lxml2
 */

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
static void print_element_names(xmlNode *a_node) {

	xmlNode *cur_node = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			printf("node type: Element, name: %s\n", cur_node->name);
		}

		//try to print properties
		xmlAttr *attribute = cur_node->properties;
		while (attribute) {
			xmlChar *value = xmlNodeListGetString(cur_node->doc,
					attribute->children, 1);
			//do something with value
			printf("attribute----- %s: %s\n", attribute->name, value);
			xmlFree(value);
			attribute = attribute->next;
		}
		//xmlFree(attribute);

		print_element_names(cur_node->children);
	}

}
/*int main(int argc, char **argv) {
	char cwd[PATH_MAX];
	   if (getcwd(cwd, sizeof(cwd)) != NULL) {
	       printf("Current working dir: %s\n", cwd);
	   } else {
	       perror("getcwd() error");
	       return 1;

	   }
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;

	// this initialize the library and check potential ABI mismatches between the version it was compiled for and the actual shared library used.
	 
	LIBXML_TEST_VERSION

	struct InsertAttack insertAttacks[MAXIMUM_INSERT_ATTACK];
	//parse the file and get the DOM
	doc = xmlReadFile(ATTACKSCENARIOXML, NULL, 0);

	if (doc == NULL) {
		printf("error: could not parse file %s\n", argv[1]);
	}

	//Get the root element node 
	root_element = xmlDocGetRootElement(doc);

	// print_element_names(root_element);
	parserAttacks(root_element);
	//free the document 
	xmlFreeDoc(doc);

	//Free the global variables that may have been allocated by the parser.
	 
	xmlCleanupParser();

	return 0;
}*/
struct AttackList getAttackList() {

	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;

	LIBXML_TEST_VERSION

	//struct InsertAttack insertAttacks[MAXIMUM_INSERT_ATTACK];
	doc = xmlReadFile(ATTACKSCENARIOXML, NULL, 0);

	if (doc == NULL) {
		printf("error: could not parse file %s\n", ATTACKSCENARIOXML);
	}

	root_element = xmlDocGetRootElement(doc);

	// print_element_names(root_element);
	struct AttackList attList = parserAttacks(root_element);
	//xmlFreeDoc(doc); todo: memory collapse happened in this line. Check it later. or call it when finish this program.
	xmlCleanupParser();

	return attList;
}

struct AttackList parserAttacks(xmlNode *a_node) {
	struct AttackList attList;
	xmlNode *cur_node = NULL;
	xmlNode *xmlAttack = a_node->children;
	for (cur_node = xmlAttack; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			if (!strcmp(cur_node->name, ATTACK_LABLE_NAME)) {
				//check attack type
				xmlAttr *attribute = cur_node->properties;
				while (attribute) {
					if (!strcmp(attribute->name, "name")) {
						xmlChar *value = xmlNodeListGetString(cur_node->doc,
								attribute->children, 1);
						if (!strcmp(value, "insertAttack")) {
							//parse as insert attack
							//printf("this is a insert attack\n");
							struct InsertAttack inAttack =parserInsertAttackXML(cur_node);
							attList.insertAttackList[attList.insertAttackNum++]=inAttack;
							/*printf("*********************final result\n");
							printf("stNum : %d\n", inAttack.stNum);
							printf("sqNum : %d\n", inAttack.sqNum);
							printf("gcbName : %s\n", inAttack.gcbName);
							printf("appId : %d\n", inAttack.appId);
							printf("dstAddress : %s\n", inAttack.dstAddress);
							printf("vlanId : %d\n", inAttack.vlanId);
							printf("vlanPriority : %d\n",
									inAttack.vlanPriority);
							printf("gocbRef : %s\n", inAttack.gocbRef);
							printf("timeAllowedtoLive : %d\n",
									inAttack.timeAllowedtoLive);
							printf("dataSet : %s\n", inAttack.dataSet);
							printf("goID : %s\n", inAttack.goID);*/


						} else if (!strcmp(value, "modifyAttack")) {
							//parse as modify attack
							//printf("this is a modify attack\n");
							struct ModifyAttack mdfAttack=parserModifyAttackXML(cur_node);
							attList.modifyAttackList[attList.modifyAttackNum++]=mdfAttack;
							/*printf("*********************modify attack final result\n");
							printf("stnum is %d\n",mdfAttack.condition_st);
							printf("sqnum is %d\n",mdfAttack.condition_sq);
							printf("gcb is %s\n",mdfAttack.condition_gcb);
							printf("time is %d\n",mdfAttack.condition_time);
							printf("arrayIndex is %d\n",mdfAttack.modifications[0].arrayIndex);
							printf("modifiedvalue is %s\n",mdfAttack.modifications[0].modifiedvalue);*/
						}
						xmlFree(value);
						break;
					}
					attribute = attribute->next;


				}
			}
		}

	}
	return attList;

}

struct InsertAttack parserInsertAttackXML(xmlNode *attackNode) {
	xmlAttr *attribute = attackNode->properties;
	struct InsertAttack insAtta;
	//printf("**********%lu\n", sizeof(insAtta));
	while (attribute) {
		if (!strcmp(attribute->name, "enable")) {
			xmlChar *value = xmlNodeListGetString(attackNode->doc,
					attribute->children, 1);
			if (!strcmp(value, "true")) {
				xmlNode *subAttackNode = attackNode->children;
				while (subAttackNode) {
					/* Start to Parse Condition Xml node*/
					if (subAttackNode->type == XML_ELEMENT_NODE
							& !strcmp(subAttackNode->name, "condition")) {
						insAtta.valid=true;
						insAtta.executed=false;
						xmlNode *conditionChild = subAttackNode->children;
						while (conditionChild) {
							if (conditionChild->type == XML_ELEMENT_NODE) {
								xmlAttr *conditionChildAttribute =conditionChild->properties;
								xmlChar *conditionChildValue;
								while (conditionChildAttribute) {
									if (!strcmp(conditionChildAttribute->name,
											"value")) {
										conditionChildValue =
												xmlNodeListGetString(
														conditionChild->doc,
														conditionChildAttribute->children,
														1);
										break;
									}
									conditionChildAttribute = conditionChildAttribute->next;
								}
								//printf("test value is %s\n",conditionChildValue);
								if (!strcmp(conditionChild->name, "stNum")) {
									insAtta.condition_st = atoi(conditionChildValue);
								} else if (!strcmp(conditionChild->name,"sqNum")) {
									insAtta.condition_sq = atoi(conditionChildValue);
								} else if (!strcmp(conditionChild->name,"gcbName")) {
									strcpy(insAtta.condition_gcb,conditionChildValue);
								} else if (!strcmp(conditionChild->name,"time")) {
									insAtta.condition_time =atof(conditionChildValue);
								}else if(!strcmp(conditionChild->name,"conditionType")){
									insAtta.condition_type = atoi(conditionChildValue);

								}
								xmlFree(conditionChildValue);
							}
							conditionChild = conditionChild->next;

						}
					}
					/* Begin to Parse payload Xml node*/
					else if (subAttackNode->type == XML_ELEMENT_NODE
							& !strcmp(subAttackNode->name, "payload")) {
						xmlNode *payloadChild = subAttackNode->children;
						while (payloadChild) {
							if (payloadChild->type == XML_ELEMENT_NODE) {
								if (!strcmp(payloadChild->name, "values")) {
									xmlNode *valueNode=payloadChild->children;
									int index=0;
									while(valueNode){
										if(valueNode->type==XML_ELEMENT_NODE){
											insAtta.values[index]=parseInsertAttackValue(valueNode);
											//printf("type is %s, value is %s\n",insAtta.values[index].type,insAtta.values[index].value);
											index++;
										}
										valueNode=valueNode->next;
									}
								} else {
									char *value = getAttributeValueByName(
											"value", payloadChild);
									if (strcmp(value, "non-value")) {
										//printf("%s value is %s\n",payloadChild->name, value);
									if(!strcmp(payloadChild->name,"stNum")) {
										insAtta.stNum=atoi(value);
									} else if(!strcmp(payloadChild->name,"sqNum")) {
										insAtta.sqNum=atoi(value);
									} else if(!strcmp(payloadChild->name,"gcbName")) {
										strcpy(insAtta.gcbName,value);
									} else if(!strcmp(payloadChild->name,"appId")) {
										insAtta.appId=atoi(value);
									} else if(!strcmp(payloadChild->name,"dstAddress")) {
										strcpy(insAtta.dstAddress,value);
									} else if(!strcmp(payloadChild->name,"vlanId")) {
										insAtta.vlanId=atoi(value);
									} else if(!strcmp(payloadChild->name,"vlanPriority")) {
										insAtta.vlanPriority=atoi(value);
									} else if(!strcmp(payloadChild->name,"gocbRef")) {
										strcpy(insAtta.gocbRef,value);
									} else if(!strcmp(payloadChild->name,"timeAllowedtoLive")) {
										insAtta.timeAllowedtoLive=atoi(value);
									} else if(!strcmp(payloadChild->name,"dataSet")) {
										strcpy(insAtta.dataSet,value);
									} else if(!strcmp(payloadChild->name,"goID")) {
										strcpy(insAtta.goID,value);
									}
								}
							}

						}
						payloadChild = payloadChild->next;
					}
					}

					subAttackNode = subAttackNode->next;
				}

			} else {
				printf("Insert Attack Disabled\n");
			}
			xmlFree(value);
			break;
		}
		attribute = attribute->next;
	}
	return insAtta;
}

char* getAttributeValueByName(char *attributeName, xmlNode *xNode) {
	xmlAttr *attribute = xNode->properties;
	xmlChar *conditionChildValue="non-value";
	while (attribute) {
		if (!strcmp(attribute->name, attributeName)) {
			conditionChildValue = xmlNodeListGetString(xNode->doc,
					attribute->children, 1);
			break;
		}
		attribute = attribute->next;
	}
	return conditionChildValue;

}
struct InsertAttackValue parseInsertAttackValue(xmlNode * valueNode){
	struct InsertAttackValue insertAttackValue;
	strcpy(insertAttackValue.type,getAttributeValueByName("type",valueNode));
	strcpy(insertAttackValue.value,getAttributeValueByName("value",valueNode));
	return insertAttackValue;

}
struct ModifyAttack parserModifyAttackXML(xmlNode *attackNode){
	xmlAttr *attribute = attackNode->properties;
		struct ModifyAttack mdfAtta;
		while (attribute) {
			if (!strcmp(attribute->name, "enable")) {
				xmlChar *value = xmlNodeListGetString(attackNode->doc,
						attribute->children, 1);
				if (!strcmp(value, "true")) {
					xmlNode *subAttackNode = attackNode->children;
					while (subAttackNode) {
						/* Start to Parse Condition Xml node*/
						if (subAttackNode->type == XML_ELEMENT_NODE
								& !strcmp(subAttackNode->name, "condition")) {
							mdfAtta.valid=true;
							mdfAtta.executed=false;
							xmlNode *conditionChild = subAttackNode->children;
							while (conditionChild) {
								if (conditionChild->type == XML_ELEMENT_NODE) {
									xmlAttr *conditionChildAttribute =conditionChild->properties;
									xmlChar *conditionChildValue;
									while (conditionChildAttribute) {
										if (!strcmp(conditionChildAttribute->name,
												"value")) {
											conditionChildValue =
													xmlNodeListGetString(
															conditionChild->doc,
															conditionChildAttribute->children,
															1);
											break;
										}
										conditionChildAttribute = conditionChildAttribute->next;
									}
									//printf("test value is %s\n",conditionChildValue);
									if (!strcmp(conditionChild->name, "stNum")) {
										mdfAtta.condition_st = atoi(conditionChildValue);
									} else if (!strcmp(conditionChild->name,"sqNum")) {
										mdfAtta.condition_sq = atoi(conditionChildValue);
									} else if (!strcmp(conditionChild->name,"gcbName")) {
										strcpy(mdfAtta.condition_gcb,conditionChildValue);
									} else if (!strcmp(conditionChild->name,"time")) {
										mdfAtta.condition_time =atof(conditionChildValue);
									}else if(!strcmp(conditionChild->name,"conditionType")){
										mdfAtta.condition_type = atoi(conditionChildValue);
									}
									xmlFree(conditionChildValue);
									//xmlFree(conditionChildAttribute);
								}
								conditionChild = conditionChild->next;

							}
						}
						/* Begin to Parse payload Xml node*/
					else if (subAttackNode->type == XML_ELEMENT_NODE
							& !strcmp(subAttackNode->name, "payload")) {
						xmlNode *payloadChild = subAttackNode->children;
						int index = 0;
						while (payloadChild) {
							if (payloadChild->type == XML_ELEMENT_NODE) {
								if (!strcmp(payloadChild->name,"modification")) {
										if (payloadChild->type== XML_ELEMENT_NODE) {
											mdfAtta.modifications[index] =parseModifyAttackValue(payloadChild);
											index++;
										}
								}
							}
							payloadChild = payloadChild->next;
						}
					}

						subAttackNode = subAttackNode->next;
					}
				} else {
					printf("Insert Attack Disabled\n");
					//return null;
				}
				xmlFree(value);
				break;
			}
			attribute = attribute->next;
		}
		return mdfAtta;
}

struct ModifyAttackModification parseModifyAttackValue(xmlNode * modificationNode){
	struct ModifyAttackModification modification;
	modification.arrayIndex=atoi(getAttributeValueByName("arrayIndex",modificationNode));
	strcpy(modification.modifiedvalue,getAttributeValueByName("modifiedValue",modificationNode));
	return modification;

}
