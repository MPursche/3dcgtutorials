#pragma once

#include <osg/NodeVisitor>
#include <osg/Node>
#include <osg/Geode>

class AddTextureUniformVisitor : public osg::NodeVisitor
{
public:
    AddTextureUniformVisitor()
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {
    }

    virtual void apply(osg::Node& node)
    {
	    // search for texture and add uniforms
        osg::StateSet* ss = node.getStateSet();

        if (ss)
        {
            ss->addUniform(new osg::Uniform("colorTexture", 0));

	        if (ss->getTextureAttributeList().size() > 0)
	        {
		        ss->addUniform(new osg::Uniform("textureActive", true));
	        } else {
		        ss->addUniform(new osg::Uniform("textureActive", false));
	        }
        }

        traverse(node);
    }

    virtual void apply(osg::Geode& geode)
    {
        for (size_t i = 0; i < geode.getNumDrawables(); ++i)
        {
            osg::Drawable* drawable = geode.getDrawable(i);
            osg::StateSet* ss = drawable->getOrCreateStateSet();
            ss->addUniform(new osg::Uniform("colorTexture", 0));

	        if (ss->getTextureAttributeList().size() > 0)
	        {
		        ss->addUniform(new osg::Uniform("textureActive", true));
	        } else {
		        ss->addUniform(new osg::Uniform("textureActive", false));
	        }            
        }

        traverse(geode);
    }
};