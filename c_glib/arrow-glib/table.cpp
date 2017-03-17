/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <arrow-glib/column.hpp>
#include <arrow-glib/schema.hpp>
#include <arrow-glib/table.hpp>

G_BEGIN_DECLS

/**
 * SECTION: table
 * @short_description: Table class
 *
 * #GArrowTable is a class for table. Table has zero or more
 * #GArrowColumns and zero or more records.
 */

typedef struct GArrowTablePrivate_ {
  std::shared_ptr<arrow::Table> table;
} GArrowTablePrivate;

enum {
  PROP_0,
  PROP_TABLE
};

G_DEFINE_TYPE_WITH_PRIVATE(GArrowTable,
                           garrow_table,
                           G_TYPE_OBJECT)

#define GARROW_TABLE_GET_PRIVATE(obj)               \
  (G_TYPE_INSTANCE_GET_PRIVATE((obj),               \
                               GARROW_TYPE_TABLE,   \
                               GArrowTablePrivate))

static void
garrow_table_dispose(GObject *object)
{
  GArrowTablePrivate *priv;

  priv = GARROW_TABLE_GET_PRIVATE(object);

  priv->table = nullptr;

  G_OBJECT_CLASS(garrow_table_parent_class)->dispose(object);
}

static void
garrow_table_set_property(GObject *object,
                          guint prop_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  GArrowTablePrivate *priv;

  priv = GARROW_TABLE_GET_PRIVATE(object);

  switch (prop_id) {
  case PROP_TABLE:
    priv->table =
      *static_cast<std::shared_ptr<arrow::Table> *>(g_value_get_pointer(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void
garrow_table_get_property(GObject *object,
                          guint prop_id,
                          GValue *value,
                          GParamSpec *pspec)
{
  switch (prop_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void
garrow_table_init(GArrowTable *object)
{
}

static void
garrow_table_class_init(GArrowTableClass *klass)
{
  GObjectClass *gobject_class;
  GParamSpec *spec;

  gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->dispose      = garrow_table_dispose;
  gobject_class->set_property = garrow_table_set_property;
  gobject_class->get_property = garrow_table_get_property;

  spec = g_param_spec_pointer("table",
                              "Table",
                              "The raw std::shared<arrow::Table> *",
                              static_cast<GParamFlags>(G_PARAM_WRITABLE |
                                                       G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property(gobject_class, PROP_TABLE, spec);
}

/**
 * garrow_table_new:
 * @name: The name of the table.
 * @schema: The schema of the table.
 * @columns: (element-type GArrowColumn): The columns of the table.
 *
 * Returns: A newly created #GArrowTable.
 */
GArrowTable *
garrow_table_new(const gchar *name,
                 GArrowSchema *schema,
                 GList *columns)
{
  std::vector<std::shared_ptr<arrow::Column>> arrow_columns;
  for (GList *node = columns; node; node = node->next) {
    GArrowColumn *column = GARROW_COLUMN(node->data);
    arrow_columns.push_back(garrow_column_get_raw(column));
  }

  auto arrow_table =
    std::make_shared<arrow::Table>(name,
                                   garrow_schema_get_raw(schema),
                                   arrow_columns);
  return garrow_table_new_raw(&arrow_table);
}

/**
 * garrow_table_get_name:
 * @table: A #GArrowTable.
 *
 * Returns: The name of the table.
 */
const gchar *
garrow_table_get_name(GArrowTable *table)
{
  const auto arrow_table = garrow_table_get_raw(table);
  return arrow_table->name().c_str();
}

/**
 * garrow_table_get_schema:
 * @table: A #GArrowTable.
 *
 * Returns: (transfer full): The schema of the table.
 */
GArrowSchema *
garrow_table_get_schema(GArrowTable *table)
{
  const auto arrow_table = garrow_table_get_raw(table);
  auto arrow_schema = arrow_table->schema();
  return garrow_schema_new_raw(&arrow_schema);
}

/**
 * garrow_table_get_column:
 * @table: A #GArrowTable.
 * @i: The index of the target column.
 *
 * Returns: (transfer full): The i-th column in the table.
 */
GArrowColumn *
garrow_table_get_column(GArrowTable *table,
                        guint i)
{
  const auto arrow_table = garrow_table_get_raw(table);
  auto arrow_column = arrow_table->column(i);
  return garrow_column_new_raw(&arrow_column);
}

/**
 * garrow_table_get_n_columns:
 * @table: A #GArrowTable.
 *
 * Returns: The number of columns in the table.
 */
guint
garrow_table_get_n_columns(GArrowTable *table)
{
  const auto arrow_table = garrow_table_get_raw(table);
  return arrow_table->num_columns();
}

/**
 * garrow_table_get_n_rows:
 * @table: A #GArrowTable.
 *
 * Returns: The number of rows in the table.
 */
guint64
garrow_table_get_n_rows(GArrowTable *table)
{
  const auto arrow_table = garrow_table_get_raw(table);
  return arrow_table->num_rows();
}

G_END_DECLS

GArrowTable *
garrow_table_new_raw(std::shared_ptr<arrow::Table> *arrow_table)
{
  auto table = GARROW_TABLE(g_object_new(GARROW_TYPE_TABLE,
                                         "table", arrow_table,
                                         NULL));
  return table;
}

std::shared_ptr<arrow::Table>
garrow_table_get_raw(GArrowTable *table)
{
  GArrowTablePrivate *priv;

  priv = GARROW_TABLE_GET_PRIVATE(table);
  return priv->table;
}
