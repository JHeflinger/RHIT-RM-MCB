#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010-2012, Fabian Greif
# Copyright (c) 2012, Sascha Schade
# Copyright (c) 2013, Martin Rosekeit
# Copyright (c) 2015, 2018, Niklas Hauser
# Copyright (c) 2016, Daniel Krebs
# Copyright (c) 2017, Michael Thies
#
# This file is part of the modm project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# -----------------------------------------------------------------------------

import os
import builder_base
import filter.cpp as filter

# -----------------------------------------------------------------------------
def filter_subtype(value):
	""" value needs to be parser.structure.SubType """
	type = filter.typeName(value.subtype.name)
	variable = filter.variableName(value.name)
	if value.subtype.isArray:
		return "%s %s[%s]" % (type, variable, value.subtype.count)
	else:
		return "%s %s" % (type, variable)

def filter_constructor(class_, default=True):
	if default:
		return "constexpr %s()" % filter.typeName(class_.name)
	else:
		parameter = []
		is_constexpr = True
		for item in class_.iter():
			type = filter.typeName(item.subtype.name)
			if item.subtype.isArray:
				type += " *"
				is_constexpr = False
			name = filter.variableName(item.name)

			parameter.append("%s %s" % (type, name))

		if len(parameter) > 0:
			return "%s%s(%s)" % ("constexpr " if is_constexpr else "", filter.typeName(class_.name), ", ".join(parameter))
		else:
			return ""

def filter_initialization_list(class_, default=True):
	initList = []
	for item in class_.iter():
		type = filter.typeName(item.subtype.name)
		name = filter.variableName(item.name)
		if item.value is not None:
			defaultValue = item.value
		else :
			defaultValue = ''

		if default:
			initList.append("%s(%s)" % (name, defaultValue))
		else:
			if item.subtype.isArray: continue;
			initList.append("%s(%s)" % (name, name))

	if len(initList):
		return ": " + ", ".join(initList)
	return ""

def filter_array_constructor(class_, default=True):
	if default: return "";

	memcpyList = []
	for item in class_.iter():
		if not item.subtype.isArray: continue;
		name = filter.variableName(item.name)
		type = filter.typeName(item.subtype.name)
		count = item.subtype.count;

		memcpyList.append("memcpy(this->%s, %s, %s * sizeof(%s));" % (name, name, count, type))

	return " ".join(memcpyList)

def filter_constexpr_constructor(class_, default=True):
	if not default:
		for item in class_.iter():
			if item.subtype.isArray: return False;

	return True

# -----------------------------------------------------------------------------
class TypeBuilder(builder_base.Builder):

	VERSION = "0.1"

	def setup(self, optparser):
		optparser.add_option(
				"--namespace",
				dest = "namespace",
				default = "robot",
				help = "Namespace of the generated identifiers.")
		optparser.add_option(
				"--source_path",
				dest = "source_path",
				default = None,
				help = "Output path for the source file")
		optparser.add_option(
				"--header_path",
				dest = "header_path",
				default = None,
				help = "Output path for the header file")
		optparser.add_option(
				"--quote_include_path",
				dest = "quote_include_path",
				default = None,
				help = "Include directive for the source file")
		optparser.add_option(
				"--system_include_path",
				dest = "system_include_path",
				default = None,
				help = "Include directive for the source file")

	def generate(self):
		# check the commandline options
		if self.options.outpath:
			source_path = self.options.outpath
			header_path = self.options.outpath
		elif self.options.source_path and self.options.header_path:
			source_path = self.options.source_path
			header_path = self.options.header_path
		else:
			raise builder_base.BuilderException("You need to provide an output path!")

		if self.options.system_include_path:
			includeDirective = '<%s>' % os.path.join(self.options.system_include_path, 'packets.hpp')
		elif self.options.quote_include_path:
			includeDirective = '"%s"' % os.path.join(self.options.system_include_path, 'packets.hpp')
		else:
			includeDirective = '"%s"' % 'packets.hpp'

		if self.options.namespace:
			namespace = self.options.namespace
		else:
			raise builder_base.BuilderException("You need to provide a namespace!")

		cppFilter = {
			'enumElement': filter.enumElement,
			'enumElementStrong': filter.typeName,
			'variableName': filter.variableName,
			'typeName': filter.typeName,
			'subtype': filter_subtype,
			'generateConstructor': filter_constructor,
			'generateArrayCopyCode': filter_array_constructor,
			'generateInitializationList': filter_initialization_list,
			'isConstexprConstructor': filter_constexpr_constructor
		}

		template_header = self.template('templates/robot_packets.hpp.tpl', filter=cppFilter)
		template_source = self.template('templates/robot_packets.cpp.tpl', filter=cppFilter)

		substitutions = {
			'components': self.tree.components,
			'actions': self.tree.components.actions,
			'events': self.tree.events,
			'packets': self.tree.types,
			'includeDirective': includeDirective,
			'namespace': namespace
		}

		file = os.path.join(header_path, 'packets.hpp')
		self.write(file, template_header.render(substitutions) + "\n")

		file = os.path.join(source_path, 'packets.cpp')
		self.write(file, template_source.render(substitutions) + "\n")

# -----------------------------------------------------------------------------
if __name__ == '__main__':
	TypeBuilder().run()
