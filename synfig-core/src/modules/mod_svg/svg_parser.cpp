/* === S Y N F I G ========================================================= */
/*!	\file svg_parser.cpp
**	\brief Implementation of the Svg parser
**	\brief Based on SVG XML specification 1.1
**	\brief See: http://www.w3.org/TR/xml11/ for deatils
**
**	$Id:$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Carlos A. Sosa Navarro
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include "svg_parser.h"

/* === U S I N G =========================================================== */

using namespace synfig;

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

Canvas::Handle
synfig::open_svg(std::string _filepath,String &errors, String &warnings){
	Canvas::Handle canvas;
	Svg_parser parser;
	try
	{
		canvas=parser.load_svg_canvas(_filepath,errors,warnings);
		//canvas->set_id(parser.get_id());
	}catch(...){
		std::cout<<"error"<<std::endl;
	}
	return canvas;
}

Canvas::Handle
Svg_parser::load_svg_canvas(std::string _filepath,String &errors, String &warnings){
	filepath = _filepath;
	#ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  	try{
  	#endif //LIBXMLCPP_EXCEPTIONS_ENABLED
		//load parser
		parser.set_substitute_entities();
		parser.parse_file(filepath);
		//set_id(filepath);
		if(parser){
		  	const xmlpp::Node* pNode = parser.get_document()->get_root_node();
		  	parser_node(pNode);
		}
	#ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  	}catch(const std::exception& ex){
    	std::cout << "Exception caught: " << ex.what() << std::endl;
  	}
  	#endif //LIBXMLCPP_EXCEPTIONS_ENABLED
	Canvas::Handle canvas;
	if(nodeRoot){
		//canvas=synfig::open_canvas(nodeRoot,_filepath,errors,warnings);
		canvas=synfig::open_canvas(nodeRoot,errors,warnings);
	}
	return canvas;
}

Svg_parser::Svg_parser(){
	uid=0;
	kux=60;
	set_canvas=0;//we must run parser_canvas method
	// 0.5 in gamma parameter of color correct layer is 1/0.5 = 2 (thinking) it must be 2.2!!!!
	gamma.set_gamma(2.2);
}
/*
String
Svg_parser::get_id(){
	if(!id_name.empty()) return id_name;
	return "id_arbitrario";
}
void
Svg_parser::set_id(String source){
	const char bad_chars[]=" :#@$^&()*";
	int inicio= 	source.find_last_of('/')+1;
	int fin=	source.find_last_of('.');
	String x=source.substr(inicio,fin-inicio);
	if(!x.empty()){
		for(unsigned int i=0;i<sizeof(bad_chars);i++){
			unsigned int pos=x.find_first_of(bad_chars[i]);
			if(pos!=String::npos)
				x.erase(pos,1);
		}
	}
	if(!x.empty()){
		id_name=x;
	}else{
		id_name="id_arbitrario";
	}
}
*/
//UPDATE

void
Svg_parser::parser_node(const xmlpp::Node* node){
  	const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  	const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  	const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  	if(nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
    	return;

  	Glib::ustring nodename = node->get_name();
  	if(!nodeText && !nodeComment && !nodename.empty()){
		if(nodename.compare("svg")==0){
			parser_svg (node);
		}else if(nodename.compare("namedview")==0){
			parser_canvas(node);
		}else if(nodename.compare("defs")==0){
			parser_defs (node);
		}else if(nodename.compare("g")==0){
			if(set_canvas==0) parser_canvas (node);
			parser_layer (node,nodeRoot->add_child("layer"),"",NULL);
			return;
		}else if(nodename.compare("rect")==0){
			if(set_canvas==0) parser_canvas (node);
			parser_rect(node,nodeRoot,"",NULL);
		}else if(nodename.compare("polygon")==0){
			if(set_canvas==0) parser_canvas (node);
			parser_polygon(node,nodeRoot,"",NULL);
		}else if(nodename.compare("path")==0){
			if(set_canvas==0) parser_canvas (node);
			parser_path (node,nodeRoot,"",NULL);
		}
  	}
  	if(!nodeContent){
    	xmlpp::Node::NodeList list = node->get_children();
    	for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter){
      		parser_node(*iter); //recursive
    	}
  	}
}

//parser elements
void
Svg_parser::parser_svg (const xmlpp::Node* node){
	//printf("un dia en algun lugar de la tierra media\n");
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		width	=etl::strprintf("%f",getDimension(nodeElement->get_attribute_value("width")));
		height	=etl::strprintf("%f",getDimension(nodeElement->get_attribute_value("height")));
		docname=nodeElement->get_attribute_value("docname","");
	}
}
void
Svg_parser::parser_canvas (const xmlpp::Node* node){
	//printf("el campo de batalla parecia un lienzo de pintura\n");
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		if(width.compare("")==0){
			width=nodeElement->get_attribute_value("width","");
		}
		if(height.compare("")==0){
			height=nodeElement->get_attribute_value("height","");
		}
		if(width.compare("")==0 && height.compare("")!=0){
			width=height;
		}
		if(width.compare("")!=0 && height.compare("")==0){
			height=width;
		}
		if(height.compare("")==0 && width.compare("")==0){
			width="1024";
			height="768";
		}
		//build
		nodeRoot=document.create_root_node("canvas", "", "");
		nodeRoot->set_attribute("version","0.5");
		nodeRoot->set_attribute("width",width);
		nodeRoot->set_attribute("height",height);
		nodeRoot->set_attribute("xres","2834.645752");
		nodeRoot->set_attribute("yres","2834.645752");
		float view_x;
		float view_y;
		view_x=atof(width.c_str())/kux;
		view_y=atof(height.c_str())/kux;
		view_x=view_x/2.0;
		view_y=view_y/2.0;
		char attr_view_box[60];
		sprintf(attr_view_box,"%f %f %f %f",-1.0*view_x,view_y,view_x,-1.0*view_y);
		nodeRoot->set_attribute("view-box",attr_view_box);
		ox=atof(width.c_str() )/2;
		oy=atof(height.c_str())/2;
		nodeRoot->set_attribute("antialias","1");
		nodeRoot->set_attribute("fps","24.000");
		nodeRoot->set_attribute("begin-time","0f");
		nodeRoot->set_attribute("end-time","5s");
		nodeRoot->set_attribute("bgcolor","0.500000 0.500000 0.500000 1.000000");
		//nodeRoot->add_child("name")->set_child_text("Synfig Animation 1");
		if(!id_name.empty()) nodeRoot->add_child("name")->set_child_text(id_name);
		else nodeRoot->add_child("name")->set_child_text("Synfig Animation 1");
	}
	set_canvas=1;
	AdjustPointUrl ();
}

void
Svg_parser::parser_rect(const xmlpp::Node* node,xmlpp::Element* root,String parent_style,Matrix* mtx_parent){
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		Glib::ustring rect_id		=nodeElement->get_attribute_value("id");
		Glib::ustring rect_style	=nodeElement->get_attribute_value("style");
		Glib::ustring rect_x		=nodeElement->get_attribute_value("x");
		Glib::ustring rect_y		=nodeElement->get_attribute_value("y");
		Glib::ustring rect_width	=nodeElement->get_attribute_value("width");
		Glib::ustring rect_height	=nodeElement->get_attribute_value("height");
		//style
		String fill			=loadAttribute("fill",rect_style,parent_style,"none");
		String fill_opacity =loadAttribute("fill-opacity",rect_style,parent_style,"1");
		String opacity		=loadAttribute("opacity",rect_style,parent_style,"1");
		//matrix
		//it's some complicated

		//build

		int typeFill=0;
		if(fill.compare(0,3,"url")==0){
			typeFill=2;
			root=nodeStartBasicLayer(root->add_child("layer"));
		}
		xmlpp::Element *child_rect=root->add_child("layer");
		child_rect->set_attribute("type","rectangle");
		child_rect->set_attribute("active","true");
		child_rect->set_attribute("version","0.2");
		child_rect->set_attribute("desc",rect_id);

		build_real(child_rect->add_child("param"),"z_depth",0.0);
		build_real(child_rect->add_child("param"),"amount",1.0);
		build_integer(child_rect->add_child("param"),"blend_method",0);
		build_color (child_rect->add_child("param"),getRed (fill),getGreen (fill),getBlue(fill),atof(opacity.data())*atof(fill_opacity.data()));

		float auxx=atof(rect_x.c_str());
		float auxy=atof(rect_y.c_str());
		coor2vect(&auxx,&auxy);
		build_vector (child_rect->add_child("param"),"point1",auxx,auxy);
		auxx= atof(rect_x.c_str()) + atof(rect_width.c_str());
		auxy= atof(rect_y.c_str()) + atof(rect_height.c_str());
		coor2vect(&auxx,&auxy);
		build_vector (child_rect->add_child("param"),"point2",auxx,auxy);
		if(typeFill==2){
			build_url (root->add_child("layer"),fill,mtx_parent);
		}
	}
}
void
Svg_parser::parser_layer(const xmlpp::Node* node,xmlpp::Element* root,String parent_style,Matrix* mtx_parent){
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		Glib::ustring label		=nodeElement->get_attribute_value("label");
		Glib::ustring style		=nodeElement->get_attribute_value("style");
		Glib::ustring fill		=nodeElement->get_attribute_value("fill");
		Glib::ustring transform	=nodeElement->get_attribute_value("transform");

		String layer_style;
		if(!style.empty()){
			layer_style=style;
		}else if(!fill.empty()){
			layer_style.append("fill:");
			layer_style.append(fill);
		}else if(!parent_style.empty()){
			layer_style=parent_style;
		}
		Matrix* mtx=NULL;
		if(!transform.empty())
			mtx=build_transform (transform);
		if(mtx_parent){
			if(mtx)
				composeMatrix(&mtx,mtx_parent,mtx);
			else
				mtx=newMatrix (mtx_parent);
		}
		//build
		root->set_attribute("type","PasteCanvas");
		root->set_attribute("active","true");
		root->set_attribute("version","0.1");
		if(!label.empty())	root->set_attribute("desc",label);
		else		root->set_attribute("desc","unknow layer");

		build_real(root->add_child("param"),"z_depth",0.0);
		build_real(root->add_child("param"),"amount",1.0);
		build_integer(root->add_child("param"),"blend_method",0);
		build_vector (root->add_child("param"),"origin",0,0);

		//printf(" atributos canvas ");
		//canvas
		xmlpp::Element *child_canvas=root->add_child("param");
		child_canvas->set_attribute("name","canvas");
		child_canvas=child_canvas->add_child("canvas");
		const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
		if(!nodeContent){
    		xmlpp::Node::NodeList list = node->get_children();
    		for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter){
				Glib::ustring name =(*iter)->get_name();
				if(name.compare("g")==0){
					parser_layer (*iter,child_canvas->add_child("layer"),layer_style,mtx);
				}else if(name.compare("path")==0){
					parser_path (*iter,child_canvas,layer_style,mtx);
				}else if(name.compare("polygon")==0){
					parser_polygon (*iter,child_canvas,layer_style,mtx);
				}else if(name.compare("rect")==0){
					parser_rect (*iter,child_canvas,layer_style,mtx);
				}
    		}
  		}
	}
}
void
Svg_parser::parser_polygon(const xmlpp::Node* node,xmlpp::Element* root,String parent_style,Matrix* mtx_parent){
	//printf("sus escudos parecian rombos y sus naves unos triangulos\n");
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		//load sub-attributes
		Glib::ustring polygon_style			=nodeElement->get_attribute_value("style");
		Glib::ustring polygon_id			=nodeElement->get_attribute_value("id");
		Glib::ustring polygon_points		=nodeElement->get_attribute_value("points");
		Glib::ustring polygon_transform		=nodeElement->get_attribute_value("transform");
		Glib::ustring polygon_fill			=nodeElement->get_attribute_value("fill");
		String fill			=loadAttribute("fill",polygon_style,parent_style,polygon_fill,"none");
		String fill_rule	=loadAttribute("fill-rule",polygon_style,parent_style,"evenodd");
		String fill_opacity	=loadAttribute("fill-opacity",polygon_style,parent_style,"1");
		String opacity		=loadAttribute("opacity",polygon_style,parent_style,"1");

		//transforms
		Matrix* mtx=NULL;
		if(!polygon_transform.empty())
			mtx=build_transform (polygon_transform);
		if(mtx_parent){
			if(mtx)
				composeMatrix(&mtx,mtx_parent,mtx);
			else
				mtx=newMatrix (mtx_parent);
		}
		//points
		if(polygon_points.empty())
			return;
		std::list<Vertice*> k;
		std::vector<String> tokens=get_tokens_path (polygon_points);
		unsigned int i;
		float ax,ay; ax=ay=0;
		for(i=0;i<tokens.size();i++){
			ax=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			ay=atof(tokens.at(i).data());
			//mtx
			if(mtx) transformPoint2D(mtx,&ax,&ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			k.push_back(nuevoVertice(ax,ay));
		}
		//escritura
		xmlpp::Element *child_polygon;

		//gradient
		int typeFill=0;
		if(fill.compare(0,3,"url")==0){
			typeFill=2;//gradient
			root=nodeStartBasicLayer(root->add_child("layer"));
		}
		child_polygon=root->add_child("layer");
		child_polygon->set_attribute("type","polygon");
		child_polygon->set_attribute("active","true");
		child_polygon->set_attribute("version","0.1");
		child_polygon->set_attribute("desc",polygon_id);
		build_param (child_polygon->add_child("param"),"z_depth","real","0.0000000000");
		build_param (child_polygon->add_child("param"),"amount","real","1.0000000000");
		build_param (child_polygon->add_child("param"),"blend_method","integer","0");
		build_color (child_polygon->add_child("param"),getRed(fill),getGreen(fill),getBlue(fill),atof(fill_opacity.data())*atof(opacity.data()));
		build_vector(child_polygon->add_child("param"),"offset",0,0);
		build_param (child_polygon->add_child("param"),"invert","bool","false");
		build_param (child_polygon->add_child("param"),"antialias","bool","true");
		build_param (child_polygon->add_child("param"),"feather","real","0.0000000000");
		build_param (child_polygon->add_child("param"),"blurtype","integer","1");
		if(fill_rule.compare("evenodd")==0) build_param (child_polygon->add_child("param"),"winding_style","integer","1");
		else build_param (child_polygon->add_child("param"),"winding_style","integer","0");
		build_points (child_polygon->add_child("param"),k);

		if(typeFill==2){
			build_url(root->add_child("layer"),fill,mtx);
		}
	}
}
void
Svg_parser::parser_path(const xmlpp::Node* node,xmlpp::Element* root,String parent_style,Matrix* mtx_parent){
	//printf("pensamos que atacarian de frente pero hicieron una curva\n");
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		//load sub-attributes
		Glib::ustring path_style		=nodeElement->get_attribute_value("style");
		Glib::ustring path_id			=nodeElement->get_attribute_value("id");
		Glib::ustring path_d			=nodeElement->get_attribute_value("d");
		Glib::ustring path_transform	=nodeElement->get_attribute_value("transform");
		Glib::ustring path_fill			=nodeElement->get_attribute_value("fill");

		String fill				=loadAttribute("fill",path_style,parent_style,path_fill,"none");
		String fill_rule		=loadAttribute("fill-rule",path_style,parent_style,"evenodd");
		String stroke			=loadAttribute("stroke",path_style,parent_style,"none");
		String stroke_width		=loadAttribute("stroke-width",path_style,parent_style,"1px");
		String stroke_linecap	=loadAttribute("stroke-linecap",path_style,parent_style,"butt");
		String stroke_linejoin	=loadAttribute("stroke-linejoin",path_style,parent_style,"miter");
		String stroke_opacity	=loadAttribute("stroke-opacity",path_style,parent_style,"1");
		String fill_opacity		=loadAttribute("fill-opacity",path_style,parent_style,"1");
		String opacity			=loadAttribute("opacity",path_style,parent_style,"1");

		//resolve transformations
		Matrix* mtx=NULL;
		if(!path_transform.empty())
			mtx=build_transform (path_transform);
		if(mtx_parent){
			if(mtx)
				composeMatrix(&mtx,mtx_parent,mtx);
			else
				mtx=newMatrix(mtx_parent);
		}
		//parser path_d attribute, this is obviously important
		std::list<std::list<Vertice*> > k;
		k=parser_path_d (path_d,mtx);

		//escribir
		int typeFill=0; //nothing
		int typeStroke=0;//nothing
		//Fill
		if(fill.compare("none")!=0){
			typeFill=1; //simple
		}
		if(typeFill==1 && fill.compare(0,3,"url")==0){
			typeFill=2;	//gradient
		}
		//Stroke
		if(stroke.compare("none")!=0){
			typeStroke=1; //simple
		}
		if(typeStroke==1 && stroke.compare(0,3,"url")==0){
			typeStroke=2;	//gradient
		}
		String bline_id;
		String offset_id;
		int n=k.size();
		if(n!=1){ //if n is > than 1 then we must create a paste canvas for all paths
			root=nodeStartBasicLayer(root->add_child("layer"));
		}
		std::list<std::list<Vertice*> >::iterator aux = k.begin();
		for (; aux!=k.end(); aux++){
			if(typeFill!=0 && typeStroke!=0){
				bline_id=new_guid();
				offset_id=new_guid();
			}
			if(typeFill==1 || typeFill==2){//region layer
				xmlpp::Element *child_fill=root;
				if(n==1 && typeFill==2){//open gradient or url (fill)
					child_fill=nodeStartBasicLayer(root->add_child("layer"));
				}
				xmlpp::Element *child_region=child_fill->add_child("layer");
				child_region->set_attribute("type","region");
				child_region->set_attribute("active","true");
				child_region->set_attribute("version","0.1");
				child_region->set_attribute("desc",path_id);
				build_param (child_region->add_child("param"),"z_depth","real","0.0000000000");
				build_param (child_region->add_child("param"),"amount","real","1.0000000000");
				build_param (child_region->add_child("param"),"blend_method","integer","0");
				build_color (child_region->add_child("param"),getRed(fill),getGreen(fill),getBlue(fill),atof(fill_opacity.data())*atof(opacity.data()));
				if(offset_id.empty())	build_vector (child_region->add_child("param"),"offset",0,0);
				else	build_vector (child_region->add_child("param"),"offset",0,0,offset_id);
				build_param (child_region->add_child("param"),"invert","bool","false");
				build_param (child_region->add_child("param"),"antialias","bool","true");
				build_param (child_region->add_child("param"),"feather","real","0.0000000000");
				build_param (child_region->add_child("param"),"blurtype","integer","1");
				if(fill_rule.compare("evenodd")==0) build_param (child_region->add_child("param"),"winding_style","integer","1");
				else build_param (child_region->add_child("param"),"winding_style","integer","0");

				build_bline (child_region->add_child("param"),*aux,loop,bline_id);

				if(n==1 && typeFill==2){ //gradient in onto mode (fill)
					build_url(child_fill->add_child("layer"),fill,mtx);
				}
			}

			if(typeStroke==1 || typeStroke==2){	//layer outline
				xmlpp::Element *child_stroke=root;
				if(n==1 && typeStroke==2){//open gradient in straigth onto (stroke)
					child_stroke=nodeStartBasicLayer(root->add_child("layer"));
				}
				xmlpp::Element *child_outline=child_stroke->add_child("layer");
				child_outline->set_attribute("type","outline");
				child_outline->set_attribute("active","true");
				child_outline->set_attribute("version","0.2");
				child_outline->set_attribute("desc",path_id);
				build_param (child_outline->add_child("param"),"z_depth","real","0.0000000000");
				build_param (child_outline->add_child("param"),"amount","real","1.0000000000");
				build_param (child_outline->add_child("param"),"blend_method","integer","0");
				build_color (child_outline->add_child("param"),getRed(stroke),getGreen(stroke),getBlue(stroke),atof(stroke_opacity.data())*atof(opacity.data()));
				if(offset_id.empty()) build_vector (child_outline->add_child("param"),"offset",0,0);
				else build_vector (child_outline->add_child("param"),"offset",0,0,offset_id);
				build_param (child_outline->add_child("param"),"invert","bool","false");
				build_param (child_outline->add_child("param"),"antialias","bool","true");
				build_param (child_outline->add_child("param"),"feather","real","0.0000000000");
				build_param (child_outline->add_child("param"),"blurtype","integer","1");
				//outline in nonzero
				build_param (child_outline->add_child("param"),"winding_style","integer","0");

				build_bline (child_outline->add_child("param"),*aux,loop,bline_id);

				stroke_width=etl::strprintf("%f",getDimension(stroke_width)/kux);
				build_param (child_outline->add_child("param"),"width","real",stroke_width);
				build_param (child_outline->add_child("param"),"expand","real","0.0000000000");
				if(stroke_linejoin.compare("miter")==0) build_param (child_outline->add_child("param"),"sharp_cusps","bool","true");
				else build_param (child_outline->add_child("param"),"sharp_cusps","bool","false");
				if(stroke_linecap.compare("butt")==0){
					build_param (child_outline->add_child("param"),"round_tip[0]","bool","false");
					build_param (child_outline->add_child("param"),"round_tip[1]","bool","false");
				}else{
					build_param (child_outline->add_child("param"),"round_tip[0]","bool","true");
					build_param (child_outline->add_child("param"),"round_tip[1]","bool","true");
				}
				build_param (child_outline->add_child("param"),"loopyness","real","1.0000000000");
				build_param (child_outline->add_child("param"),"homogeneous_width","bool","true");

				if(n==1 && typeStroke==2){ //gradient in onto mode (stroke)
					build_url(child_stroke->add_child("layer"),stroke,mtx);
				}
			}
		}
		if(n!=1){//only fill for several canvas in one path
			if(typeFill==2){
				build_url(root->add_child("layer"),fill,mtx);
			}
		}
	}
}

std::vector<String>
Svg_parser::get_tokens_path(String path){ //mini path lexico-parser
	std::vector<String> tokens;
	String buffer;
	int e=0;
	unsigned int i=0;
	char a;
	while(i<path.size()){
		a=path.at(i);
		switch(e){
			case 0: //initial state
					if(a=='m'){ e=1; i++;}
					else if(a=='c'){ e= 2; i++;}
					else if(a=='q'){ e= 3; i++;}
					else if(a=='t'){ e= 4; i++;}
					else if(a=='a'){ e= 5; i++;}
					else if(a=='l'){ e= 6; i++;}
					else if(a=='v'){ e= 7; i++;}
					else if(a=='h'){ e= 8; i++;}
					else if(a=='M'){ e= 9; i++;}
					else if(a=='C'){ e=10; i++;}
					else if(a=='Q'){ e=11; i++;}
					else if(a=='T'){ e=12; i++;}
					else if(a=='A'){ e=13; i++;}
					else if(a=='L'){ e=14; i++;}
					else if(a=='V'){ e=15; i++;}
					else if(a=='H'){ e=16; i++;}
					else if(a=='z' || a=='Z'){ e=17; i++;}
					else if(a=='-' ||a=='.'|| isdigit (a)){ e=18;}
					else if(a==','){ e=19; i++;}
					else if(a==' '){i++;}
					break;
			//relative
			case 1 : tokens.push_back("m"); e=0; break;//move
			case 2 : tokens.push_back("c"); e=0; break;//curve
			case 3 : tokens.push_back("q"); e=0; break;//quadratic
			case 4 : tokens.push_back("t"); e=0; break;//smooth quadratic
			case 5 : tokens.push_back("a"); e=0; break;//elliptic arc
			case 6 : tokens.push_back("l"); e=0; break;//line to
			case 7 : tokens.push_back("v"); e=0; break;//vertical
			case 8 : tokens.push_back("h"); e=0; break;//horizontal
			//absolute
			case 9 : tokens.push_back("M"); e=0; break;
			case 10: tokens.push_back("C"); e=0; break;
			case 11: tokens.push_back("Q"); e=0; break;
			case 12: tokens.push_back("T"); e=0; break;
			case 13: tokens.push_back("A"); e=0; break;
			case 14: tokens.push_back("L"); e=0; break;
			case 15: tokens.push_back("V"); e=0; break;
			case 16: tokens.push_back("H"); e=0; break;

			case 17: tokens.push_back("z"); e=0; break;//loop
			case 18: if(a=='-'||a=='.'|| isdigit (a)){
						buffer.append(path.substr(i,1));i++;
					}else{
						e=20;
					}
					break;
			case 19: tokens.push_back(","); e=0; break;
			case 20: tokens.push_back(buffer);
					buffer.clear();
					e=0; break;
			default: break;
		}
	}
	switch(e){//last element
		case 1 : tokens.push_back("m"); break;
		case 2 : tokens.push_back("c"); break;
		case 3 : tokens.push_back("q"); break;
		case 4 : tokens.push_back("t"); break;
		case 5 : tokens.push_back("a"); break;
		case 6 : tokens.push_back("l"); break;
		case 7 : tokens.push_back("v"); break;
		case 8 : tokens.push_back("h"); break;
		case 9 : tokens.push_back("M"); break;
		case 10: tokens.push_back("C"); break;
		case 11: tokens.push_back("Q"); break;
		case 12: tokens.push_back("T"); break;
		case 13: tokens.push_back("A"); break;
		case 14: tokens.push_back("L"); break;
		case 15: tokens.push_back("V"); break;
		case 16: tokens.push_back("H"); break;
		case 17: tokens.push_back("z"); break;
		case 18: tokens.push_back(buffer); break;
		case 19: tokens.push_back(","); break;
		case 20: tokens.push_back(buffer); break;
		default: break;
	}
	return tokens;
}

std::list<std::list<Vertice*> >
Svg_parser::parser_path_d(String path_d,Matrix* mtx){
	std::list<std::list<Vertice*> > k;
	std::list<Vertice*> k1;
	float ax,ay,tgx,tgy,tgx2,tgy2;//each method
	float actual_x=0,actual_y=0;//for relative methods;
	loop=false;
	unsigned int i;
	std::vector<String> tokens=get_tokens_path(path_d);
	for(i=0;i<tokens.size();i++){
		if(tokens.at(i).compare("M")==0){//absolute move to
			if(!k1.empty())
				k.push_front(k1);
			k1.clear();
			//read
			i++; ax=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			ay=atof(tokens.at(i).data());
			actual_x=ax;
			actual_y=ay;
			//operate and save
			if(mtx) transformPoint2D(mtx,&ax,&ay);
			coor2vect(&ax,&ay);
			k1.push_back(nuevoVertice (ax,ay)); //first element
			setSplit(k1.back(),TRUE);
		}else if(tokens.at(i).compare("C")==0){ //absolute curve
			//tg2
			i++; tgx2=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			tgy2=atof(tokens.at(i).data());
			//tg1
			i++; tgx=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			tgy=atof(tokens.at(i).data());
			//point
			i++; ax=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			ay=atof(tokens.at(i).data());
			actual_x=ax;
			actual_y=ay;
			//mtx
			if(mtx){
				transformPoint2D(mtx,&tgx2,&tgy2);
				transformPoint2D(mtx,&ax,&ay);
				transformPoint2D(mtx,&tgx,&tgy);
			}
			//ajust
			coor2vect(&tgx2,&tgy2);
			coor2vect(&ax,&ay);
			coor2vect(&tgx,&tgy);
			//save
			setTg2(k1.back(),k1.back()->x,k1.back()->y,tgx2,tgy2);
			if(isFirst(k1.front(),ax,ay)){
				setTg1(k1.front(),k1.front()->x,k1.front()->y,tgx,tgy);
			}else{
				k1.push_back(nuevoVertice (ax,ay));
				setTg1(k1.back(),k1.back()->x,k1.back()->y,tgx,tgy);
				setSplit(k1.back(),TRUE);
			}
		}else if(tokens.at(i).compare("Q")==0){ //absolute quadractic curve
			//tg1 and tg2
			i++; tgx=ax=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			tgy=ay=atof(tokens.at(i).data());
			//point
			i++; ax=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			ay=atof(tokens.at(i).data());
			actual_x=ax;
			actual_y=ay;
			//mtx
			if(mtx){
				transformPoint2D(mtx,&ax,&ay);
				transformPoint2D(mtx,&tgx,&tgy);
			}
			//adjust
			coor2vect(&ax,&ay);
			coor2vect(&tgx,&tgy);
			//save
			setTg1(k1.back(),k1.back()->x,k1.back()->y,tgx,tgy);
			setSplit(k1.back(),FALSE);
			k1.push_back(nuevoVertice (ax,ay));
			setTg1(k1.back(),k1.back()->x,k1.back()->y,tgx,tgy);
		}else if(tokens.at(i).compare("L")==0){ //absolute line to
			//point
			i++; ax=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			ay=atof(tokens.at(i).data());
			actual_x=ax;
			actual_y=ay;
			//mtx
			if(mtx) transformPoint2D(mtx,&ax,&ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			setTg2(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			if(isFirst(k1.front(),ax,ay)){
				setTg1(k1.front(),k1.front()->x,k1.front()->y,k1.front()->x,k1.front()->y);
			}else{
				k1.push_back(nuevoVertice(ax,ay));
				setTg1(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			}
		}else if(tokens.at(i).compare("l")==0){//relative line to
			//point read
			i++; ax=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			ay=atof(tokens.at(i).data());
			//relative
			ax=actual_x+ax;
			ay=actual_y+ay;
			actual_x=ax;
			actual_y=ay;
			//mtx
			if(mtx) transformPoint2D(mtx,&ax,&ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			setTg2(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			if(isFirst(k1.front(),ax,ay)){
				setTg1(k1.front(),k1.front()->x,k1.front()->y,k1.front()->x,k1.front()->y);
			}else{
				k1.push_back(nuevoVertice(ax,ay));
				setTg1(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			}
		}else if(tokens.at(i).compare("H")==0){//absolute horizontal move
			//the same that L but only Horizontal movement
			//point
			i++; ax=atof(tokens.at(i).data());
			ay=actual_y;
			actual_x=ax;
			actual_y=ay;
			//mtx
			if(mtx) transformPoint2D(mtx,&ax,&ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			setTg2(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			if(isFirst(k1.front(),ax,ay)){
				setTg1(k1.front(),k1.front()->x,k1.front()->y,k1.front()->x,k1.front()->y);
			}else{
				k1.push_back(nuevoVertice(ax,ay));
				setTg1(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			}
		}else if(tokens.at(i).compare("h")==0){//horizontal relative
			i++; ax=atof(tokens.at(i).data());
			ax=actual_x+ax;
			ay=actual_y;
			actual_x=ax;
			actual_y=ay;
			//mtx
			if(mtx) transformPoint2D(mtx,&ax,&ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			setTg2(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			if(isFirst(k1.front(),ax,ay)){
				setTg1(k1.front(),k1.front()->x,k1.front()->y,k1.front()->x,k1.front()->y);
			}else{
				k1.push_back(nuevoVertice(ax,ay));
				setTg1(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			}
		}else if(tokens.at(i).compare("V")==0){//vertical absolute
			//point
			i++; ay=atof(tokens.at(i).data());
			ax=actual_x;
			actual_x=ax;
			actual_y=ay;
			//mtx
			if(mtx) transformPoint2D(mtx,&ax,&ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			setTg2(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			if(isFirst(k1.front(),ax,ay)){
				setTg1(k1.front(),k1.front()->x,k1.front()->y,k1.front()->x,k1.front()->y);
			}else{
				k1.push_back(nuevoVertice(ax,ay));
				setTg1(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			}
		}else if(tokens.at(i).compare("v")==0){//relative
			//point
			i++; ay=atof(tokens.at(i).data());
			ax=actual_x;
			ay=actual_y+ay;
			actual_x=ax;
			actual_y=ay;
			//mtx
			if(mtx) transformPoint2D(mtx,&ax,&ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			setTg2(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			if(isFirst(k1.front(),ax,ay)){
				setTg1(k1.front(),k1.front()->x,k1.front()->y,k1.front()->x,k1.front()->y);
			}else{
				k1.push_back(nuevoVertice(ax,ay));
				setTg1(k1.back(),k1.back()->x,k1.back()->y,k1.back()->x,k1.back()->y);
			}
		}else if(tokens.at(i).compare("T")==0){// I don't know what does it
		}else if(tokens.at(i).compare("A")==0){//elliptic arc

			//isn't complete support, is only for circles

			//this curve have 6 parameters
			//radio
			float radio_x,radio_y;
			float angle;
			bool sweep,large;
			//radio
			i++; radio_x=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			radio_y=atof(tokens.at(i).data());
			//angle
			i++; angle=atof(tokens.at(i).data());
			//flags
			i++; large=atoi(tokens.at(i).data());
			i++; sweep=atoi(tokens.at(i).data());
			//point
			i++; ax=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			ay=atof(tokens.at(i).data());
			//how to draw?
			if(!large && !sweep){
				//points
				tgx2 = actual_x + radio_x*0.5;
				tgy2 = actual_y ;
				tgx  = ax;
				tgy  = ay + radio_y*0.5;
				actual_x=ax;
				actual_y=ay;
				//transformations
				if(mtx){
					transformPoint2D(mtx,&tgx2,&tgy2);
					transformPoint2D(mtx,&ax,&ay);
					transformPoint2D(mtx,&tgx,&tgy);
				}
				//adjust
				coor2vect(&tgx2,&tgy2);
				coor2vect(&ax,&ay);
				coor2vect(&tgx,&tgy);
				//save
				setTg2(k1.back(),k1.back()->x,k1.back()->y,tgx2,tgy2);
				if(isFirst(k1.front(),ax,ay)){
					setTg1(k1.front(),k1.front()->x,k1.front()->y,tgx,tgy);
				}else{
					k1.push_back(nuevoVertice (ax,ay));
					setTg1(k1.back(),k1.back()->x,k1.back()->y,tgx,tgy);
					setSplit(k1.back(),TRUE);
				}
			}else if(!large &&  sweep){
				//points
				tgx2 = actual_x;
				tgy2 = actual_y + radio_y*0.5;
				tgx  = ax + radio_x*0.5;
				tgy  = ay ;
				actual_x=ax;
				actual_y=ay;
				//transformations
				if(mtx){
					transformPoint2D(mtx,&tgx2,&tgy2);
					transformPoint2D(mtx,&ax,&ay);
					transformPoint2D(mtx,&tgx,&tgy);
				}
				//adjust
				coor2vect(&tgx2,&tgy2);
				coor2vect(&ax,&ay);
				coor2vect(&tgx,&tgy);
				//save
				setTg2(k1.back(),k1.back()->x,k1.back()->y,tgx2,tgy2);
				if(isFirst(k1.front(),ax,ay)){
					setTg1(k1.front(),k1.front()->x,k1.front()->y,tgx,tgy);
				}else{
					k1.push_back(nuevoVertice (ax,ay));
					setTg1(k1.back(),k1.back()->x,k1.back()->y,tgx,tgy);
					setSplit(k1.back(),TRUE);
				}
			}else if( large && !sweep){//rare
				//this need more than one vertex
			}else if( large &&  sweep){//circles in inkscape are made with this kind of arc
				if(actual_y==ay){//circles
					//intermediate point
					int sense=1;
					if(actual_x>ax) sense =-1;
					float in_x,in_y,in_tgx1,in_tgy1,in_tgx2,in_tgy2;
					in_x = (actual_x+ax)/2;
					in_y = actual_y - sense*radio_y;
					in_tgx1 = in_x - sense*(radio_x*0.5);
					in_tgx2 = in_x + sense*(radio_x*0.5);
					in_tgy1 = in_y;
					in_tgy2 = in_y;
					//start/end points
					tgx2=actual_x;
					tgy2=ay - sense*(radio_y*0.5);
					tgx =ax;
					tgy =ay - sense*(radio_y*0.5);

					actual_x=ax;
					actual_y=ay;
					//transformations
					if(mtx){
						transformPoint2D(mtx,&tgx2,&tgy2);
						transformPoint2D(mtx,&tgx ,&tgy );
						transformPoint2D(mtx,&ax,&ay);

						transformPoint2D(mtx,&in_tgx2,&in_tgy2);
						transformPoint2D(mtx,&in_tgx1,&in_tgy1);
						transformPoint2D(mtx,&in_x,&in_y);
					}
					//adjust
					coor2vect(&tgx2 , &tgy2);
					coor2vect(&ax   , &ay  );
					coor2vect(&tgx  , &tgy );

					coor2vect(&in_tgx2 , &in_tgy2);
					coor2vect(&in_tgx1 , &in_tgy1);
					coor2vect(&in_x    , &in_y   );

					//save the last tg2
					setTg2(k1.back(),k1.back()->x,k1.back()->y,tgx2,tgy2);
					//save the intermediate point
					k1.push_back(nuevoVertice (in_x,in_y));
					setTg1(k1.back(),k1.back()->x,k1.back()->y, in_tgx1 , in_tgy1);
					setTg2(k1.back(),k1.back()->x,k1.back()->y, in_tgx2 , in_tgy2);
					setSplit(k1.back(),TRUE); //this could be changed
					//save the new point
					if(isFirst(k1.front(),ax,ay)){
						setTg1(k1.front(),k1.front()->x,k1.front()->y,tgx,tgy);
					}else{
						k1.push_back(nuevoVertice (ax,ay));
						setTg1(k1.back(),k1.back()->x,k1.back()->y,tgx,tgy);
						setSplit(k1.back(),TRUE);
					}
				}
			}
		}else if(tokens.at(i).compare("z")==0){
			loop=true;
		}else{
			std::cout<<"don't supported: "<<tokens.at(i)<<std::endl;
		}
	}
	if(!k1.empty())
		k.push_front(k1); //last element
	return k;
}

void
Svg_parser::parser_defs(const xmlpp::Node* node){
	const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
	if(!nodeContent){
		xmlpp::Node::NodeList list = node->get_children();
		for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter){
			Glib::ustring name =(*iter)->get_name();
			if(name.compare("linearGradient")==0){
				parser_linearGradient(*iter);
			}else if(name.compare("radialGradient")==0){
				parser_radialGradient(*iter);
			}
 		}
  	}
}
void
Svg_parser::AdjustPointUrl(){
/*
	if(!lg.empty()){//linealgradient
		std::list<LinearGradient*>::iterator aux=lg.begin();
		while(aux!=lg.end()){
			LinearGradient* auxlg=*aux;
			coor2vect (&auxlg->x1,&auxlg->y1);
			coor2vect (&auxlg->x2,&auxlg->y2);
			aux++;
		}
	}
	if(!rg.empty()){//radialgradient
		std::list<RadialGradient*>::iterator aux=rg.begin();
		while(aux!=rg.end()){
			RadialGradient* auxrg=*aux;
			coor2vect (&auxrg->cx,&auxrg->cy);
			auxrg->r= auxrg->r/kux;
			aux++;
		}
	}
*/
}
std::list<ColorStop*>*
Svg_parser::find_colorStop(String name){
	if(!name.empty()){
		if(lg.empty()&& rg.empty())
			return NULL;

		String buscar= name;
		if(buscar.at(0)=='#') buscar.erase(0,1);
		else return NULL;
		std::list<LinearGradient*>::iterator aux=lg.begin();
		while(aux!=lg.end()){//only find into linear gradients
			if(buscar.compare((*aux)->name)==0){
				return (*aux)->stops;
			}
			aux++;
		}
	}
	return NULL;
}
void
Svg_parser::build_url(xmlpp::Element* root, String name,Matrix *mtx){
	if(!name.empty()){
		if(lg.empty()&& rg.empty())
			root->get_parent()->remove_child(root);

		int inicio=name.find_first_of("#")+1;
		int fin=name.find_first_of(")");
		String buscar= name.substr(inicio,fin-inicio);
		bool encontro=false;
		if(!lg.empty()){
			std::list<LinearGradient*>::iterator aux=lg.begin();
			while(aux!=lg.end()){
				if(buscar.compare((*aux)->name)==0){
					build_linearGradient (root,*aux,mtx);
					encontro=true;
				}
				aux++;
			}
		}
		if(!encontro && !rg.empty()){
			std::list<RadialGradient*>::iterator aux=rg.begin();
			while(aux!=rg.end()){
				if(buscar.compare((*aux)->name)==0){
					build_radialGradient (root,*aux,mtx);
					encontro=true;
				}
				aux++;
			}
		}
		if(!encontro)
			root->get_parent()->remove_child(root);
	}else{
		root->get_parent()->remove_child(root);
	}
}
void
Svg_parser::build_stop_color(xmlpp::Element* root, std::list<ColorStop*> *stops){
	std::list<ColorStop*>::iterator aux_stop=stops->begin();
	while(aux_stop!=stops->end()){
		xmlpp::Element *child=root->add_child("color");
		child->set_attribute("pos",etl::strprintf("%f",(*aux_stop)->pos));
		child->add_child("r")->set_child_text(etl::strprintf("%f",(*aux_stop)->r));
		child->add_child("g")->set_child_text(etl::strprintf("%f",(*aux_stop)->g));
		child->add_child("b")->set_child_text(etl::strprintf("%f",(*aux_stop)->b));
		child->add_child("a")->set_child_text(etl::strprintf("%f",(*aux_stop)->a));
		aux_stop++;
	}
}
void
Svg_parser::build_linearGradient(xmlpp::Element* root,LinearGradient* data,Matrix* mtx){
	if(data){
		root->set_attribute("type","linear_gradient");
		root->set_attribute("active","true");
		root->set_attribute("desc","Gradient004");
		build_param (root->add_child("param"),"z_depth","real","0");
		build_param (root->add_child("param"),"amount","real","1");
		//straight onto
		build_param (root->add_child("param"),"blend_method","integer","21");
		float x1,y1,x2,y2;
		x1=data->x1;
		y1=data->y1;
		x2=data->x2;
		y2=data->y2;
		if(mtx){
			transformPoint2D(mtx,&x1,&y1);
			transformPoint2D(mtx,&x2,&y2);
		}
		coor2vect (&x1,&y1);
		coor2vect (&x2,&y2);

		build_vector (root->add_child("param"),"p1",x1,y1);
		build_vector (root->add_child("param"),"p2",x2,y2);
		//gradient link
		xmlpp::Element *child=root->add_child("param");
		child->set_attribute("name","gradient");
		build_stop_color (child->add_child("gradient"),data->stops);
		build_param (root->add_child("param"),"loop","bool","false");
		build_param (root->add_child("param"),"zigzag","bool","false");
	}
}
void
Svg_parser::build_radialGradient(xmlpp::Element* root,RadialGradient* data,Matrix* mtx){
//not completed
	if(data){
		root->set_attribute("type","radial_gradient");
		root->set_attribute("active","true");
		build_param (root->add_child("param"),"z_depth","real","0");
		build_param (root->add_child("param"),"amount","real","1");
		//straight onto
		build_param (root->add_child("param"),"blend_method","integer","21");
		//gradient link
		xmlpp::Element *child=root->add_child("param");
		child->set_attribute("name","gradient");
		build_stop_color (child->add_child("gradient"),data->stops);
		//here the center point and radio
		float cx=data->cx;
		float cy=data->cy;
		float r =data->r;
		//transform
		if(mtx){
			transformPoint2D(mtx,&cx,&cy);
		}
		//adjust
		coor2vect (&cx,&cy);
		r=r/kux;
		build_vector (root->add_child("param"),"center",cx,cy);
		build_param (root->add_child("param"),"radius","real",r);

		build_param (root->add_child("param"),"loop","bool","false");
		build_param (root->add_child("param"),"zigzag","bool","false");
	}
}

void
Svg_parser::parser_linearGradient(const xmlpp::Node* node){
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		Glib::ustring id	=nodeElement->get_attribute_value("id");
		float x1			=atof(nodeElement->get_attribute_value("x1").data());
		float y1			=atof(nodeElement->get_attribute_value("y1").data());
		float x2			=atof(nodeElement->get_attribute_value("x2").data());
		float y2			=atof(nodeElement->get_attribute_value("y2").data());
		Glib::ustring link	=nodeElement->get_attribute_value("href");

		std::list<ColorStop*> *stops;
		if(!link.empty()){
			stops=find_colorStop (link);
		}else{
			//color stops
			stops=new std::list<ColorStop*>();
			const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
			if(!nodeContent){
    			xmlpp::Node::NodeList list = node->get_children();
    			for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter){
					Glib::ustring name =(*iter)->get_name();
					if(name.compare("stop")==0){
						const xmlpp::Element* nodeIter = dynamic_cast<const xmlpp::Element*>(*iter);
						Glib::ustring style	=nodeIter->get_attribute_value("style");
						float offset=atof(nodeIter->get_attribute_value("offset").data());
						String stop_color;
						String opacity;
						if(!style.empty()){
							extractSubAttribute (style,"stop-color",&stop_color);
							extractSubAttribute (style,"stop-opacity",&opacity);
						}
						if(opacity.empty()) opacity="1";
						if(stop_color.empty()) stop_color="#000000";//black for default :S
						stops->push_back(newColorStop(stop_color,atof(opacity.data()),offset));
					}
    			}
			}
		}
		if(stops)
			lg.push_back(newLinearGradient(id,x1,y1,x2,y2,stops));
	}
}

void
Svg_parser::parser_radialGradient(const xmlpp::Node* node){
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		Glib::ustring id	=nodeElement->get_attribute_value("id");
		float cx			=atof(nodeElement->get_attribute_value("cx").data());
		float cy			=atof(nodeElement->get_attribute_value("cy").data());
		float r				=atof(nodeElement->get_attribute_value("r").data());
		Glib::ustring link	=nodeElement->get_attribute_value("href");//basic
		std::list<ColorStop*> *stops=NULL;
		if(!link.empty()){
			//inkscape always use link, i dont need parser stops here, but it's posible
			stops=find_colorStop (link);
		}
		if(stops)
			rg.push_back(newRadialGradient(id,cx,cy,r,stops));
	}
}

ColorStop*
Svg_parser::newColorStop(String color,float opacity,float pos){
	ColorStop* _stop;
	_stop=(ColorStop*)malloc(sizeof(ColorStop));
	float r=getRed(color);
	float g=getGreen(color);
	float b=getBlue(color);
	float a=opacity;
	Color ret=adjustGamma(r/255,g/255,b/255,a);
	_stop->r=ret.get_r();
	_stop->g=ret.get_g();
	_stop->b=ret.get_b();
	_stop->a=ret.get_a();
	_stop->pos=pos;
	return _stop;
}
Color
Svg_parser::adjustGamma(float r,float g,float b,float a){
	Color ret(r,g,b,a);
	if(gamma.get_gamma_r()!=1.0){
		if(ret.get_r() < 0)
			ret.set_r(-gamma.r_F32_to_F32(-ret.get_r()));
		else
			ret.set_r(gamma.r_F32_to_F32(ret.get_r()));
	}
	if(gamma.get_gamma_g()!=1.0){
		if(ret.get_g() < 0)
			ret.set_g(-gamma.g_F32_to_F32(-ret.get_g()));
		else
			ret.set_g(gamma.g_F32_to_F32(ret.get_g()));
	}
	if(gamma.get_gamma_b()!=1.0){
		if(ret.get_b() < 0)
			ret.set_b(-gamma.b_F32_to_F32(-ret.get_b()));
		else
			ret.set_b(gamma.b_F32_to_F32(ret.get_b()));
	}
	return ret;
}

LinearGradient*
Svg_parser::newLinearGradient(String name,float x1,float y1, float x2,float y2,std::list<ColorStop*> *stops){
	LinearGradient* data;
	data=(LinearGradient*)malloc(sizeof(LinearGradient));
	sprintf(data->name,"%s",name.data());
	data->x1=x1;
	data->y1=y1;
	data->x2=x2;
	data->y2=y2;
	data->stops=stops;
   	return data;
}

RadialGradient*
Svg_parser::newRadialGradient(String name,float cx,float cy,float r,std::list<ColorStop*> *stops){
	RadialGradient* data;
	data=(RadialGradient*)malloc(sizeof(RadialGradient));
	sprintf(data->name,"%s",name.data());
	data->cx=cx;
	data->cy=cy;
	data->r=r;
	data->stops=stops;
	return data;
}

//builds
void
Svg_parser::build_gamma(xmlpp::Element* root,float gamma){
	root->set_attribute("type","colorcorrect");
	root->set_attribute("active","true");
	root->set_attribute("version","0.1");
	root->set_attribute("desc","Gamma");
	build_real (root->add_child("param"),"gamma",gamma);
}
Matrix*
Svg_parser::build_transform(const String transform){
	Matrix* a=NULL;
	String tf(transform);
	removeIntoS(&tf);
	std::vector<String> tokens=tokenize(tf," ");
	std::vector<String>::iterator aux=tokens.begin();
	while(aux!=tokens.end()){
		if((*aux).compare(0,9,"translate")==0){
			float dx,dy;
			int inicio,fin;
			inicio	=(*aux).find_first_of("(")+1;
			fin		=(*aux).find_first_of(",");
			dx		=atof((*aux).substr(inicio,fin-inicio).data());
			inicio	=(*aux).find_first_of(",")+1;
			fin		=(*aux).size()-1;
			dy		=atof((*aux).substr(inicio,fin-inicio).data());
			if(matrixVacia(a))
				a=newMatrix(1,0,0,1,dx,dy);
			else
				multiplyMatrix(&a,newMatrix(1,0,0,1,dx,dy));
		}else if((*aux).compare(0,5,"scale")==0){
			if(matrixVacia(a))
				a=newMatrix(1,0,0,1,0,0);
		}else if((*aux).compare(0,6,"rotate")==0){
			float angle,seno,coseno;
			int inicio,fin;
			inicio	=(*aux).find_first_of("(")+1;
			fin		=(*aux).size()-1;
			angle=getRadian (atof((*aux).substr(inicio,fin-inicio).data()));
			seno   =sin(angle);
			coseno =cos(angle);
			if(matrixVacia(a))
				a=newMatrix(coseno,seno,-1*seno,coseno,0,0);
			else
				multiplyMatrix(&a,newMatrix(coseno,seno,-1*seno,coseno,0,0));
		}else if((*aux).compare(0,6,"matrix")==0){
			int inicio	=(*aux).find_first_of('(')+1;
			int fin		=(*aux).find_first_of(')');
			if(matrixVacia(a))
				a=newMatrix((*aux).substr(inicio,fin-inicio));
			else
				multiplyMatrix(&a,newMatrix((*aux).substr(inicio,fin-inicio)));
		}else{
			a=newMatrix(1,0,0,1,0,0);
		}
		aux++;
	}
	return a;
}

void
Svg_parser::build_translate(xmlpp::Element* root,float dx,float dy){
	root->set_attribute("type","translate");
	root->set_attribute("active","true");
	root->set_attribute("version","0.1");
	build_vector (root->add_child("param"),"origin",dx,dy);
}
void
Svg_parser::build_rotate(xmlpp::Element* root,float dx,float dy,float angle){
	root->set_attribute("type","rotate");
	root->set_attribute("active","true");
	root->set_attribute("version","0.1");
	build_vector (root->add_child("param"),"origin",dx,dy);
	build_real   (root->add_child("param"),"amount",angle);
}
void
Svg_parser::build_points(xmlpp::Element* root,std::list<Vertice*> p){
	root->set_attribute("name","vector_list");
	xmlpp::Element *child=root->add_child("dynamic_list");
	child->set_attribute("type","vector");
	std::list<Vertice*>::iterator aux = p.begin();
	while(aux!=p.end()){
		xmlpp::Element *child_entry=child->add_child("entry");
		xmlpp::Element *child_vector=child_entry->add_child("vector");
		child_vector->add_child("x")->set_child_text(etl::strprintf("%f",(*aux)->x));
		child_vector->add_child("y")->set_child_text(etl::strprintf("%f",(*aux)->y));
		aux++;
	}
}
void
Svg_parser::build_vertice(xmlpp::Element* root , Vertice *p){
	xmlpp::Element *child_comp=root->add_child("composite");
	child_comp->set_attribute("type","bline_point");
	build_vector (child_comp->add_child("param"),"point",p->x,p->y);
	build_param (child_comp->add_child("width"),"","real","1.0000000000");
	build_param (child_comp->add_child("origin"),"","real","0.5000000000");
	if(p->split) build_param (child_comp->add_child("split"),"","bool","true");
	else build_param (child_comp->add_child("split"),"","bool","false");
	//tangente 1
	xmlpp::Element *child_t1=child_comp->add_child("t1");
	xmlpp::Element *child_rc=child_t1->add_child("radial_composite");
	child_rc->set_attribute("type","vector");
	build_param (child_rc->add_child("radius"),"","real",p->radio1);
	build_param (child_rc->add_child("theta"),"","angle",p->angle1);
	//tangente 2
	xmlpp::Element *child_t2=child_comp->add_child("t2");
	xmlpp::Element *child_rc2=child_t2->add_child("radial_composite");
	child_rc2->set_attribute("type","vector");
	build_param (child_rc2->add_child("radius"),"","real",p->radio2);
	build_param (child_rc2->add_child("theta"),"","angle",p->angle2);

}
void
Svg_parser::build_bline(xmlpp::Element* root,std::list<Vertice*> p,bool loop,String blineguid){
	root->set_attribute("name","bline");
	xmlpp::Element *child=root->add_child("bline");
	child->set_attribute("type","bline_point");
	if(loop)
		child->set_attribute("loop","true");
	else
		child->set_attribute("loop","false");
	if(!blineguid.empty())	child->set_attribute("guid",blineguid);
	std::list<Vertice*>::iterator aux = p.begin();
	while(aux!=p.end()){
		if(*aux) build_vertice (child->add_child("entry"),*aux);
		aux++;
	}
}

void
Svg_parser::build_param(xmlpp::Element* root,String name,String type,String value){
	if(!type.empty() && !value.empty()){
		if(!name.empty())	root->set_attribute("name",name);
		xmlpp::Element *child=root->add_child(type);
		child->set_attribute("value",value);
	}else{
		root->get_parent()->remove_child(root);
	}
}
void
Svg_parser::build_param(xmlpp::Element* root,String name,String type,float value){
	if(!type.empty()){
		if(!name.empty()) root->set_attribute("name",name);
		xmlpp::Element *child=root->add_child(type);
		child->set_attribute("value",etl::strprintf ("%f",value));
	}else{
		root->get_parent()->remove_child(root);
	}
}
void
Svg_parser::build_param(xmlpp::Element* root,String name,String type,int value){
	if(!type.empty()){
			if(!name.empty()) root->set_attribute("name",name);
			xmlpp::Element *child=root->add_child(type);
			char *enteroc=new char[10];
			sprintf(enteroc,"%d",value);
			child->set_attribute("value",enteroc);
			delete [] enteroc;
	}else{
		root->get_parent()->remove_child(root);
	}
}

void
Svg_parser::build_integer(xmlpp::Element* root,String name,int value){
	if(name.compare("")!=0) root->set_attribute("name",name);
	xmlpp::Element *child=root->add_child("integer");
	char *enteroc=new char[10];
	sprintf(enteroc,"%d",value);
	child->set_attribute("value",enteroc);
}
void
Svg_parser::build_real(xmlpp::Element* root,String name,float value){
	if(name.compare("")!=0) root->set_attribute("name",name);
	xmlpp::Element *child=root->add_child("real");
	char *realc=new char[20];
	sprintf(realc,"%f",value);
	child->set_attribute("value",realc);
}

void
Svg_parser::build_color(xmlpp::Element* root,float r,float g,float b,float a){
	if(r>255 || g>255 || b>255 || a>1 || r<0 || g<0 || b<0 || a<0){
		root->get_parent()->remove_child(root);
		printf("Color aborted\n");
		return;
	}
	Color ret=adjustGamma(r/255,g/255,b/255,a);

	root->set_attribute("name","color");
	xmlpp::Element *child=root->add_child("color");
	child->add_child("r")->set_child_text(etl::strprintf("%f",ret.get_r()));
	child->add_child("g")->set_child_text(etl::strprintf("%f",ret.get_g()));
	child->add_child("b")->set_child_text(etl::strprintf("%f",ret.get_b()));
	child->add_child("a")->set_child_text(etl::strprintf("%f",ret.get_a()));
}
void
Svg_parser::build_vector(xmlpp::Element* root,String name,float x,float y){

	if(name.compare("")!=0) root->set_attribute("name",name);
	xmlpp::Element *child=root->add_child("vector");
	child->add_child("x")->set_child_text(etl::strprintf("%f",x));
	child->add_child("y")->set_child_text(etl::strprintf("%f",y));

}
void
Svg_parser::build_vector (xmlpp::Element* root,String name,float x,float y,String guid){
	if(name.compare("")!=0) root->set_attribute("name",name);
	xmlpp::Element *child=root->add_child("vector");
	if(!guid.empty()) child->set_attribute("guid",guid);
	child->add_child("x")->set_child_text(etl::strprintf("%f",x));
	child->add_child("y")->set_child_text(etl::strprintf("%f",y));
}

xmlpp::Element*
Svg_parser::nodeStartBasicLayer(xmlpp::Element* root){
	root->set_attribute("type","PasteCanvas");
	root->set_attribute("active","true");
	root->set_attribute("version","0.1");
	root->set_attribute("desc","Composite");
	build_param (root->add_child("param"),"z_depth","real","0");
	build_param (root->add_child("param"),"amount","real","1");
	build_param (root->add_child("param"),"blend_method","integer","0");
	build_vector (root->add_child("param"),"origin",0,0);
	xmlpp::Element *child=root->add_child("param");
	child->set_attribute("name","canvas");
	return child->add_child("canvas");
}

//metodos extras
void
Svg_parser::coor2vect(float *x,float *y){
	float sx, sy;
	sx=*x;
	sy=*y;
	sy= atof(height.c_str())-sy;
	sx= sx - ox;
	sy= sy - oy;
	sx= sx / kux;
	sy= sy / kux;
	*x=sx; *y=sy;
}

void
Svg_parser::setTg1(Vertice *p,float p1x,float p1y,float p2x,float p2y){
	float rd=0,ag=0;
	float d1x,d1y,d2x,d2y,dx,dy;
	d1x=p1x*60;
	d1y=p1y*60;
	d2x=p2x*60;
	d2y=p2y*60;
	dx=d2x-d1x;
	dy=d2y-d1y;
	dx=dx*3;
	dy=dy*3;
	dx=dx/60;
	dy=dy/60;
	rd=sqrt(dx*dx + dy*dy);
	if(dx>0 && dy>0){
		ag=PI + atan(dy/dx);
	}else if(dx>0 && dy<0){
		ag=PI + atan(dy/dx);
	}else if(dx<0 && dy<0){
		ag=atan(dy/dx);
	}else if(dx<0 && dy>0){
		ag= 2*PI+atan(dy/dx);
	}else if(dx==0 && dy>0){
		ag=-1*PI/2;
	}else if(dx==0 && dy<0){
		ag=PI/2;
	}else if(dx==0 && dy==0){
		ag=0;
	}else if(dx<0 && dy==0){
		ag=0;
	}else if(dx>0 && dy==0){
		ag=PI;
	}
	ag= (ag*180)/PI;
	p->radio1=rd;
	p->angle1=ag;
}
void
Svg_parser::setTg2(Vertice* p,float p1x,float p1y,float p2x,float p2y){
	float rd=0,ag=0;
	float d1x,d1y,d2x,d2y,dx,dy;
	d1x=p1x*60;
	d1y=p1y*60;
	d2x=p2x*60;
	d2y=p2y*60;
	dx=d2x-d1x;
	dy=d2y-d1y;
	dx=dx*3;
	dy=dy*3;
	dx=dx/60;
	dy=dy/60;

	rd=sqrt(dx*dx + dy*dy);
	if(dx>0 && dy>0){
		ag=PI + atan(dy/dx);
	//	printf("caso 180-270\n");
	}else if(dx>0 && dy<0){
		ag=PI + atan(dy/dx);
	//	printf("caso 90-180\n");
	}else if(dx<0 && dy<0){
		ag=atan(dy/dx);
	//	printf("caso 0-90\n");
	}else if(dx<0 && dy>0){
		ag= 2*PI+atan(dy/dx);
	//	printf("caso 270-360\n");
	}else if(dx==0 && dy>0){
		ag=-1*PI/2;
	}else if(dx==0 && dy<0){
		ag=PI/2;
	}else if(dx==0 && dy==0){
		ag=0;
	}else if(dx<0 && dy==0){
		ag=0;
	}else if(dx>0 && dy==0){
		ag=PI;
	}
	ag= (ag*180)/PI;
	ag=ag-180;
	p->radio2=rd;
	p->angle2=ag;
}

void
Svg_parser::setSplit(Vertice* p,bool val){
	if(p!=NULL){
		p->split=val;
	}
}
int
Svg_parser::isFirst(Vertice* nodo,float a, float b){
	if(nodo->x==a && nodo->y==b)
		return 1;
	return 0;
}

Vertice*
Svg_parser::nuevoVertice(float x,float y){
	Vertice* nuevo;
	nuevo=(Vertice*)malloc(sizeof(Vertice));
	nuevo->x=x;
	nuevo->y=y;
	nuevo->radio1=nuevo->radio2=nuevo->angle1=nuevo->angle2=0;
	return nuevo;
}

int
Svg_parser::extractSubAttribute(const String attribute, String name,String* value){
	int encontro=0;
	if(!attribute.empty()){
		String str(attribute);
		removeS(&str);
		std::vector<String> tokens=tokenize(str,";");
		std::vector<String>::iterator aux=tokens.begin();
		while(aux!=tokens.end()){
			int medio= (*aux).find_first_of(":");
			if((*aux).substr(0,medio).compare(name)==0){
				int fin=(*aux).size();
				*value=(*aux).substr(medio+1,fin-medio);
				return 1;
			}
			aux++;
		}
	}
	return encontro;
}
String
Svg_parser::loadAttribute(String name,const String path_style,const String master_style,const String defaultVal){
	String value;
	int fnd=0;
	if(!path_style.empty())
		fnd=extractSubAttribute(path_style,name,&value);
	if(fnd==0){
		if(!master_style.empty())
			fnd=extractSubAttribute(master_style,name,&value);
		if(fnd==0)
			value=defaultVal;
	}
	return value;
}
String
Svg_parser::loadAttribute(String name,const String path_style,const String master_style,const String subattribute,const String defaultVal){
	String value;
	int fnd=0;
	if(!path_style.empty())
		fnd=extractSubAttribute(path_style,name,&value);
	if(fnd==0 && !master_style.empty())
			fnd=extractSubAttribute(master_style,name,&value);
	if(fnd==0){
		if(!subattribute.empty())
			value=subattribute;
		else
			value=defaultVal;
	}
	return value;
}

int
Svg_parser::randomLetter(){
	int a=rand()%2;
	if(a) return (49 + rand()%9);
	else return  (65 + rand()%24);
}

int
Svg_parser::getRed(String hex){
	if(hex.at(0)=='#'){
		return hextodec(hex.substr(1,2));
	}else if(hex.compare(0,3,"rgb")==0 || hex.compare(0,3,"RGB")==0){
		int inicio=hex.find_first_of("(")+1;
		int fin	=hex.find_last_of(")");
		String aux=tokenize(hex.substr(inicio,fin-inicio),",").at(0);
		return atoi(aux.data());
	}
	return 0;
}
int
Svg_parser::getGreen(String hex){
	if(hex.at(0)=='#'){
		return hextodec(hex.substr(3,2));
	}else if(hex.compare(0,3,"rgb")==0 || hex.compare(0,3,"RGB")==0){
		int inicio=hex.find_first_of("(")+1;
		int fin	=hex.find_last_of(")");
		String aux=tokenize(hex.substr(inicio,fin-inicio),",").at(1);
		return atoi(aux.data());
	}
	return 0;
}
int
Svg_parser::getBlue(String hex){
	if(hex.at(0)=='#'){
		return hextodec(hex.substr(5,2));
	}else if(hex.compare(0,3,"rgb")==0 || hex.compare(0,3,"RGB")==0){
		int inicio=hex.find_first_of("(")+1;
		int fin	=hex.find_last_of(")");
		String aux=tokenize(hex.substr(inicio,fin-inicio),",").at(2);
		return atoi(aux.data());
	}
	return 0;
}
int
Svg_parser::hextodec(String hex){
	int result=0;
	if(!hex.empty()){
		int top=hex.size();
		int ihex[top];
		int i=0;
		while(i<top){
			if(hex.at(i)=='0')
				ihex[i]=0;
			else if(hex.at(i)=='1')
				ihex[i]=1;
			else if(hex.at(i)=='2')
				ihex[i]=2;
			else if(hex.at(i)=='3')
				ihex[i]=3;
			else if(hex.at(i)=='4')
				ihex[i]=4;
			else if(hex.at(i)=='5')
				ihex[i]=5;
			else if(hex.at(i)=='6')
				ihex[i]=6;
			else if(hex.at(i)=='7')
				ihex[i]=7;
			else if(hex.at(i)=='8')
				ihex[i]=8;
			else if(hex.at(i)=='9')
				ihex[i]=9;
			else if(hex.at(i)=='a')
				ihex[i]=10;
			else if(hex.at(i)=='b')
				ihex[i]=11;
			else if(hex.at(i)=='c')
				ihex[i]=12;
			else if(hex.at(i)=='d')
				ihex[i]=13;
			else if(hex.at(i)=='e')
				ihex[i]=14;
			else if(hex.at(i)=='f')
				ihex[i]=15;
			else
				return 0;
			i++;
		}
		i=0;
		while(i<top){
			result+=pow(16,i)*ihex[top-i-1];
			i++;
		}
	}
	return result;
}

float
Svg_parser::getDimension(const String ac){
	if(ac.empty()){
		return 0;
	}
	int length=ac.size();
	float af=0;
	if(isdigit(ac.at(length-1))){
		af=atof(ac.data());
	}else if(ac.at(length-1)=='%'){
			return 1024;
	}else{
		String mtc=ac.substr(length-2,length);
		String nmc=ac.substr(0,length-2);
		if(mtc.compare("px")==0){
			af=atof(nmc.data());
		}else if(mtc.compare("pt")==0){
			af=atof(nmc.data())*1.25;
		}else if(mtc.compare("em")==0){
			af=atof(nmc.data())*16;
		}else if(mtc.compare("mm")==0){
			af=atof(nmc.data())*3.54;
		}else if(mtc.compare("pc")==0){
			af=atof(nmc.data())*15;
		}else if(mtc.compare("cm")==0){
			af=atof(nmc.data())*35.43;
		}else if(mtc.compare("in")==0){
			af=atof(nmc.data())*90;
		}else{
			return 1024;
		}
	}
	return af;
}
//matrix operations
Matrix*
Svg_parser::newMatrix(Matrix *a){
	Matrix* data;
	data=(Matrix*)malloc(sizeof(Matrix));
	data->a=a->a;		data->b=a->b;		data->c=a->c;
	data->d=a->d;		data->e=a->e;		data->f=a->f;
	return data;
}
Matrix*
Svg_parser::newMatrix(float a,float b,float c,float d,float e,float f){
	Matrix* data;
	data=(Matrix*)malloc(sizeof(Matrix));
	data->a=a;		data->b=b;		data->c=c;
	data->d=d;		data->e=e;		data->f=f;
	return data;
}
Matrix*
Svg_parser::newMatrix(const String mvector){
	if(!mvector.empty()){
		Matrix* data=(Matrix*)malloc(sizeof(Matrix));
		std::vector<String> tokens=tokenize(mvector,",");
		if(tokens.size()!=6) return newMatrix(1,0,0,1,0,0);
		data->a=atof(tokens.at(0).data());
		data->b=atof(tokens.at(1).data());
		data->c=atof(tokens.at(2).data());
		data->d=atof(tokens.at(3).data());
		data->e=atof(tokens.at(4).data());
		data->f=atof(tokens.at(5).data());
		return data;
	}else{
		return newMatrix(1,0,0,1,0,0);
	}
}
void
Svg_parser::transformPoint2D(Matrix *mtx,float *a,float *b){
	float auxa,auxb;
	auxa=0;
	auxb=0;
	auxa= (*a)*(mtx->a) + (*b)*(mtx->c) + (mtx->e);
	auxb= (*a)*(mtx->b) + (*b)*(mtx->d) + (mtx->f);
	*a=auxa;
	*b=auxb;
	return;
}
void
Svg_parser::composeMatrix(Matrix **mtx,Matrix* mtx1,Matrix* mtx2){
	Matrix* aux=newMatrix(0,0,0,0,0,0);
	aux->a=(mtx1->a)*(mtx2->a)+(mtx1->c)*(mtx2->b);
	aux->b=(mtx1->b)*(mtx2->a)+(mtx1->d)*(mtx2->b);
	aux->c=(mtx1->a)*(mtx2->c)+(mtx1->c)*(mtx2->d);
	aux->d=(mtx1->b)*(mtx2->c)+(mtx1->d)*(mtx2->d);
	aux->e=(mtx1->a)*(mtx2->e)+(mtx1->c)*(mtx2->f)+(mtx1->e);
	aux->f=(mtx1->b)*(mtx2->e)+(mtx1->d)*(mtx2->f)+(mtx1->f);
	*mtx=aux;
}
void
Svg_parser::multiplyMatrix(Matrix **mtx1,Matrix *mtx2){
	Matrix* aux=newMatrix(0,0,0,0,0,0);
	aux->a=((*mtx1)->a)*(mtx2->a)+((*mtx1)->c)*(mtx2->b);
	aux->b=((*mtx1)->b)*(mtx2->a)+((*mtx1)->d)*(mtx2->b);
	aux->c=((*mtx1)->a)*(mtx2->c)+((*mtx1)->c)*(mtx2->d);
	aux->d=((*mtx1)->b)*(mtx2->c)+((*mtx1)->d)*(mtx2->d);
	aux->e=((*mtx1)->a)*(mtx2->e)+((*mtx1)->c)*(mtx2->f)+((*mtx1)->e);
	aux->f=((*mtx1)->b)*(mtx2->e)+((*mtx1)->d)*(mtx2->f)+((*mtx1)->f);
	(*mtx1)->a=aux->a;
	(*mtx1)->b=aux->b;
	(*mtx1)->c=aux->c;
	(*mtx1)->d=aux->d;
	(*mtx1)->e=aux->e;
	(*mtx1)->f=aux->f;
}
bool
Svg_parser::matrixVacia(Matrix *mtx){
	if(mtx == NULL) return true;
	return false;
}

float
Svg_parser::getRadian(float sexa){
	return (sexa*2*PI)/360;
}
void
Svg_parser::removeS(String *input){
	for(unsigned int i=0;i<input->size();i++){
		if(input->at(i)==' '){
			input->erase(i,1);
		}
	}
}
void
Svg_parser::removeIntoS(String *input){
	bool into=false;
	for(unsigned int i=0;i<input->size();i++){
		if(input->at(i)=='('){
			into=true;
		}else if(input->at(i)==')'){
			into=false;
		}else if(into && input->at(i)==' '){
			input->erase(i,1);
		}
	}
}
std::vector<String>
Svg_parser::tokenize(const String& str,const String& delimiters){
	std::vector<String> tokens;
	String::size_type lastPos = str.find_first_not_of(delimiters, 0);
	String::size_type pos = str.find_first_of(delimiters, lastPos);
	while (String::npos != pos || String::npos != lastPos){
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}
	return tokens;
}
String
Svg_parser::new_guid(){
	uid++;
	return GUID::hasher(uid).get_string();
}
