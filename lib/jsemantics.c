/*
 * Copyright (c) 2010-2013 Michael Kuhn
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * \file
 **/

#include <julea-config.h>

#include <glib.h>

#include <string.h>

#include <jsemantics.h>

/**
 * \defgroup JSemantics Semantics
 * @{
 **/

/**
 * A semantics object.
 **/
struct JSemantics
{
	gint atomicity;

	/**
	 * The consistency semantics.
	 **/
	gint consistency;

	/**
	 * The persistency semantics.
	 **/
	gint persistency;

	/**
	 * The concurrency semantics.
	 **/
	gint concurrency;

	/**
	 * The safety semantics.
	 **/
	gint safety;

	/**
	 * The security semantics.
	 **/
	gint security;

	/**
	 * Whether the semantics object is immutable.
	 **/
	gboolean immutable;

	/**
	 * The reference count.
	 **/
	gint ref_count;
};

/**
 * Creates a new semantics object.
 * Semantics objects become immutable after the first call to j_semantics_ref().
 *
 * \author Michael Kuhn
 *
 * \code
 * \endcode
 *
 * \return A new semantics object. Should be freed with j_semantics_unref().
 **/
JSemantics*
j_semantics_new (JSemanticsTemplate template)
{
	JSemantics* semantics;

	semantics = g_slice_new(JSemantics);
	semantics->atomicity = J_SEMANTICS_ATOMICITY_NONE;
	semantics->concurrency = J_SEMANTICS_CONCURRENCY_OVERLAPPING;
	semantics->consistency = J_SEMANTICS_CONSISTENCY_IMMEDIATE;
	semantics->persistency = J_SEMANTICS_PERSISTENCY_EVENTUAL;
	semantics->safety = J_SEMANTICS_SAFETY_NONE;
	semantics->security = J_SEMANTICS_SECURITY_STRICT;
	semantics->immutable = FALSE;
	semantics->ref_count = 1;

	switch (template)
	{
		case J_SEMANTICS_TEMPLATE_DEFAULT:
			break;
		case J_SEMANTICS_TEMPLATE_POSIX:
			semantics->atomicity = J_SEMANTICS_ATOMICITY_OPERATION;
			semantics->concurrency = J_SEMANTICS_CONCURRENCY_OVERLAPPING;
			semantics->consistency = J_SEMANTICS_CONSISTENCY_IMMEDIATE;
			semantics->persistency = J_SEMANTICS_PERSISTENCY_EVENTUAL;
			semantics->safety = J_SEMANTICS_SAFETY_NONE;
			semantics->security = J_SEMANTICS_SECURITY_STRICT;
			break;
		case J_SEMANTICS_TEMPLATE_CHECKPOINT:
			semantics->atomicity = J_SEMANTICS_ATOMICITY_NONE;
			semantics->concurrency = J_SEMANTICS_CONCURRENCY_NON_OVERLAPPING;
			semantics->consistency = J_SEMANTICS_CONSISTENCY_EVENTUAL;
			semantics->persistency = J_SEMANTICS_PERSISTENCY_EVENTUAL;
			semantics->safety = J_SEMANTICS_SAFETY_NONE;
			semantics->security = J_SEMANTICS_SECURITY_NONE;
			break;
		default:
			g_warn_if_reached();
	}

	return semantics;
}

/**
 * Increases the semantics' reference count.
 *
 * \author Michael Kuhn
 *
 * \code
 * \endcode
 *
 * \param semantics The semantics.
 *
 * \return The semantics.
 **/
JSemantics*
j_semantics_ref (JSemantics* semantics)
{
	g_return_val_if_fail(semantics != NULL, NULL);

	g_atomic_int_inc(&(semantics->ref_count));

	if (!semantics->immutable)
	{
		semantics->immutable = TRUE;
	}

	return semantics;
}

/**
 * Decreases the semantics' reference count.
 * When the reference count reaches zero, frees the memory allocated for the semantics.
 *
 * \author Michael Kuhn
 *
 * \code
 * \endcode
 *
 * \param semantics The semantics.
 **/
void
j_semantics_unref (JSemantics* semantics)
{
	g_return_if_fail(semantics != NULL);

	if (g_atomic_int_dec_and_test(&(semantics->ref_count)))
	{
		g_slice_free(JSemantics, semantics);
	}
}

/**
 * Sets a specific aspect of the semantics.
 *
 * \author Michael Kuhn
 *
 * \code
 * JSemantics* semantics;
 * ...
 * j_semantics_set(semantics, J_SEMANTICS_PERSISTENCY, J_SEMANTICS_PERSISTENCY_LAX);
 * \endcode
 *
 * \param semantics The semantics.
 * \param key       The aspect's key.
 * \param value     The aspect's value.
 **/
void
j_semantics_set (JSemantics* semantics, JSemanticsType key, gint value)
{
	g_return_if_fail(semantics != NULL);
	g_return_if_fail(!semantics->immutable);

	switch (key)
	{
		case J_SEMANTICS_ATOMICITY:
			semantics->atomicity = value;
			break;
		case J_SEMANTICS_CONCURRENCY:
			semantics->concurrency = value;
			break;
		case J_SEMANTICS_CONSISTENCY:
			semantics->consistency = value;
			break;
		case J_SEMANTICS_PERSISTENCY:
			semantics->persistency = value;
			break;
		case J_SEMANTICS_SAFETY:
			semantics->safety = value;
			break;
		case J_SEMANTICS_SECURITY:
			semantics->security = value;
			break;
		default:
			g_warn_if_reached();
	}
}

/**
 * Gets a specific aspect of the semantics.
 *
 * \author Michael Kuhn
 *
 * \code
 * JSemantics* semantics;
 * ...
 * j_semantics_get(semantics, J_SEMANTICS_PERSISTENCY);
 * \endcode
 *
 * \param semantics The semantics.
 * \param key       The aspect's key.
 *
 * \return The aspect's value.
 **/
gint
j_semantics_get (JSemantics* semantics, JSemanticsType key)
{
	g_return_val_if_fail(semantics != NULL, -1);

	switch (key)
	{
		case J_SEMANTICS_ATOMICITY:
			return semantics->atomicity;
		case J_SEMANTICS_CONCURRENCY:
			return semantics->concurrency;
		case J_SEMANTICS_CONSISTENCY:
			return semantics->consistency;
		case J_SEMANTICS_PERSISTENCY:
			return semantics->persistency;
		case J_SEMANTICS_SAFETY:
			return semantics->safety;
		case J_SEMANTICS_SECURITY:
			return semantics->security;
		default:
			g_return_val_if_reached(-1);
	}
}

/* Internal */

JSemantics*
j_semantics_parse (gchar const* template_str, gchar const* semantics_str)
{
	JSemantics* semantics;
	gchar** parts;
	guint parts_len;

	if (g_strcmp0(template_str, "posix") == 0)
	{
		semantics = j_semantics_new(J_SEMANTICS_TEMPLATE_POSIX);
	}
	else if (g_strcmp0(template_str, "checkpoint") == 0)
	{
		semantics = j_semantics_new(J_SEMANTICS_TEMPLATE_CHECKPOINT);
	}
	else
	{
		semantics = j_semantics_new(J_SEMANTICS_TEMPLATE_DEFAULT);
	}

	if (semantics_str == NULL)
	{
		return semantics;
	}

	parts = g_strsplit(semantics_str, ",", 0);
	parts_len = g_strv_length(parts);

	for (guint i = 0; i < parts_len; i++)
	{
		gchar const* value;

		if ((value = strchr(parts[i], '=')) == NULL)
		{
			continue;
		}

		value++;

		if (g_str_has_prefix(parts[i], "atomicity="))
		{
			if (g_strcmp0(value, "batch") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_ATOMICITY, J_SEMANTICS_ATOMICITY_BATCH);
			}
			else if (g_strcmp0(value, "operation") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_ATOMICITY, J_SEMANTICS_ATOMICITY_OPERATION);
			}
			else if (g_strcmp0(value, "none") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_ATOMICITY, J_SEMANTICS_ATOMICITY_NONE);
			}
		}
		else if (g_str_has_prefix(parts[i], "concurrency="))
		{
			if (g_strcmp0(value, "overlapping") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_CONCURRENCY, J_SEMANTICS_CONCURRENCY_OVERLAPPING);
			}
			else if (g_strcmp0(value, "non-overlapping") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_CONCURRENCY, J_SEMANTICS_CONCURRENCY_NON_OVERLAPPING);
			}
			else if (g_strcmp0(value, "none") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_CONCURRENCY, J_SEMANTICS_CONCURRENCY_NONE);
			}
		}
		else if (g_str_has_prefix(parts[i], "consistency="))
		{
			if (g_strcmp0(value, "immediate") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_CONSISTENCY, J_SEMANTICS_CONSISTENCY_IMMEDIATE);
			}
			else if (g_strcmp0(value, "eventual") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_CONSISTENCY, J_SEMANTICS_CONSISTENCY_EVENTUAL);
			}
			else if (g_strcmp0(value, "none") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_CONSISTENCY, J_SEMANTICS_CONSISTENCY_NONE);
			}
		}
		else if (g_str_has_prefix(parts[i], "persistency="))
		{
			if (g_strcmp0(value, "immediate") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_PERSISTENCY, J_SEMANTICS_PERSISTENCY_IMMEDIATE);
			}
			else if (g_strcmp0(value, "eventual") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_PERSISTENCY, J_SEMANTICS_PERSISTENCY_EVENTUAL);
			}
			else if (g_strcmp0(value, "none") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_PERSISTENCY, J_SEMANTICS_PERSISTENCY_NONE);
			}
		}
		else if (g_str_has_prefix(parts[i], "safety="))
		{
			if (g_strcmp0(value, "storage") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_SAFETY, J_SEMANTICS_SAFETY_STORAGE);
			}
			else if (g_strcmp0(value, "network") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_SAFETY, J_SEMANTICS_SAFETY_NETWORK);
			}
			else if (g_strcmp0(value, "none") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_SAFETY, J_SEMANTICS_SAFETY_NONE);
			}
		}
		else if (g_str_has_prefix(parts[i], "security="))
		{
			if (g_strcmp0(value, "strict") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_SECURITY, J_SEMANTICS_SECURITY_STRICT);
			}
			else if (g_strcmp0(value, "none") == 0)
			{
				j_semantics_set(semantics, J_SEMANTICS_SECURITY, J_SEMANTICS_SECURITY_NONE);
			}
		}
	}

	g_strfreev(parts);

	return semantics;
}

/**
 * @}
 **/
