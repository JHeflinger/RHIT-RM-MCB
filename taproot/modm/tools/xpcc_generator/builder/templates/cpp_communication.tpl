%# Copyright (c) 2013-2014, Georgi Grinshpun
%# Copyright (c) 2015, Niklas Hauser
%# Copyright (c) 2015, Sascha Schade
%#
%# This file is part of the modm project.
%#
%# This Source Code Form is subject to the terms of the Mozilla Public
%# License, v. 2.0. If a copy of the MPL was not distributed with this
%# file, You can obtain one at http://mozilla.org/MPL/2.0/.
%# ----------------------------------------------------------------------------
/*
 * WARNING: This file is generated automatically from cpp_communication.tpl.
 * Do not edit! Please modify the corresponding XML file instead.
 */
// ----------------------------------------------------------------------------

#ifndef {{ namespace | upper }}_CPP_COMMUNICATION_HPP
#define {{ namespace | upper }}_CPP_COMMUNICATION_HPP

#include "identifier.hpp"
#include "packets.hpp"
#include <modm/communication/xpcc/communicator.hpp>

namespace {{ namespace }}
{

	/** Using this class you can publish events.*/
	class EventPublisher
	{
	public:

	{%- for event in events.iter() %}
		{% if event.type -%}
		{% if event.description %}/** {{ event.description | modm.wordwrap(72) | modm.indent(2) }}
		* \param const {{ namespace }}::packet::{{ event.type.name | CamelCase -}}& */{% endif %}
		static inline void
		{{ event.name | camelCase }}(
				xpcc::Communicator *communicator,

				{%- if event.type.isBuiltIn %}
				const {{ event.type.name | CamelCase }}& packet)
				{%- else %}
				const {{ namespace }}::packet::{{ event.type.name | CamelCase }}& packet)
				{%- endif %}
		{
			communicator->publishEvent(
				{{ namespace }}::event::Identifier::{{ event.name | CAMELCASE }},
				packet);
		}
		{% else -%}
		{% if event.description %}/** {{ event.description | modm.wordwrap(72) | modm.indent(2) }}*/{% endif %}
		static inline void
		{{ event.name | camelCase }}(xpcc::Communicator *communicator) {
			communicator->publishEvent(
				{{ namespace }}::event::Identifier::{{ event.name | CAMELCASE }});
		}
		{% endif -%}
	{% endfor %}
	};

{%- for component in components.iter() %}

	{% if component.description %}/** {{ component.description | modm.wordwrap(72) | modm.indent(2) }} */{% endif %}
	class {{ component.name | CamelCase }}
	{
	public:
		{% for action in component.flattened().actions %}
		{% if action.description -%}
		{% if action.parameterType -%}
		/** {{ action.description | modm.wordwrap(72) | modm.indent(2) }}
		\param packet {{ namespace }}::packet::{{ action.parameterType.flattened().name | CamelCase }}&
		 */
		{% else -%}
		/** {{ action.description | modm.wordwrap(72) | modm.indent(2) }} */
		{% endif -%}
		{% else -%}
		{% if action.parameterType -%}
		/**
		\param packet {{ namespace }}::packet::{{ action.parameterType.flattened().name | CamelCase }}&
		 */
		{% endif -%}
		{% endif -%}
		{% if action.parameterType -%}
		static inline void
		{{ action.name | camelCase }} (
				xpcc::Communicator *communicator,
				{%- if action.parameterType.isBuiltIn %}
				const {{ action.parameterType.name | CamelCase }}& packet)
				{%- else %}
				const {{ namespace }}::packet::{{ action.parameterType.name | CamelCase }}& packet)
				{%- endif %}
		{
			communicator->callAction(
				{{ namespace }}::component::Identifier::{{ component.name | CAMELCASE }},
				{{ namespace }}::action::Identifier::{{ action.name | CAMELCASE }},
				packet);
		}
		static inline void
		{{ action.name | camelCase }} (
				xpcc::Communicator *communicator,
				{%- if action.parameterType.isBuiltIn %}
				const {{ action.parameterType.name | CamelCase }}& packet,
				{%- else %}
				const {{ namespace }}::packet::{{ action.parameterType.name | CamelCase }}& packet,
				{%- endif %}
				xpcc::ResponseCallback& responseCallback)
		{
			communicator->callAction(
				{{ namespace }}::component::Identifier::{{ component.name | CAMELCASE }},
				{{ namespace }}::action::Identifier::{{ action.name | CAMELCASE }},
				packet,
				responseCallback);
		}
		{% else -%}
		static inline void
		{{ action.name | camelCase }} (xpcc::Communicator *communicator) {
			communicator->callAction(
				{{ namespace }}::component::Identifier::{{ component.name | CAMELCASE }},
				{{ namespace }}::action::Identifier::{{ action.name | CAMELCASE }});
		}
		static inline void
		{{ action.name | camelCase }} (xpcc::Communicator *communicator, xpcc::ResponseCallback& responseCallback) {
			communicator->callAction(
				{{ namespace }}::component::Identifier::{{ component.name | CAMELCASE }},
				{{ namespace }}::action::Identifier::{{ action.name | CAMELCASE }},
				responseCallback);
		}
		{% endif %}
		{%- endfor %}

	};


{%- endfor %}

} // namespace {{ namespace }}

#endif // {{ namespace | upper }}_CPP_COMMUNICATION_HPP
