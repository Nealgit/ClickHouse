#pragma once

#include <Common/escapeForFileName.h>
#include <Common/quoteString.h>
#include <Databases/DatabasesCommon.h>
#include <Interpreters/Context.h>
#include <Parsers/ASTCreateQuery.h>
#include <Storages/IStorage.h>


namespace DB
{

ASTPtr parseCreateQueryFromMetadataFile(const String & filepath, Poco::Logger * log);

std::pair<String, StoragePtr> createTableFromAST(
    ASTCreateQuery ast_create_query,
    const String & database_name,
    const String & table_data_path_relative,
    Context & context,
    bool has_force_restore_data_flag);

/** Get the row with the table definition based on the CREATE query.
  * It is an ATTACH query that you can execute to create a table from the correspondent database.
  * See the implementation.
  */
String getObjectDefinitionFromCreateQuery(const ASTPtr & query);


/* Class to provide basic operations with tables when metadata is stored on disk in .sql files.
 */
class DatabaseOnDisk : public DatabaseWithOwnTablesBase
{
public:
    DatabaseOnDisk(const String & name, const String & metadata_path_, const String & logger)
    : DatabaseWithOwnTablesBase(name, logger)
    , metadata_path(metadata_path_)
    , data_path("data/" + escapeForFileName(database_name) + "/") {}

    void createTable(
        const Context & context,
        const String & table_name,
        const StoragePtr & table,
        const ASTPtr & query) override;

    void removeTable(
        const Context & context,
        const String & table_name) override;

    void renameTable(
        const Context & context,
        const String & table_name,
        IDatabase & to_database,
        const String & to_table_name,
        TableStructureWriteLockHolder & lock) override;

    ASTPtr getCreateDatabaseQuery(const Context & context) const override;

    void drop(const Context & context) override;

    String getObjectMetadataPath(const String & object_name) const override;

    time_t getObjectMetadataModificationTime(const String & object_name) const override;

    String getDataPath() const override { return data_path; }
    String getDataPath(const String & table_name) const override;
    String getDataPath(const ASTCreateQuery & query) const override;
    String getMetadataPath() const override { return metadata_path; }

protected:
    using IteratingFunction = std::function<void(const String &)>;
    void iterateMetadataFiles(const Context & context, const IteratingFunction & iterating_function) const;

    ASTPtr getCreateTableQueryImpl(
        const Context & context,
        const String & table_name,
        bool throw_on_error) const override;

    String getDatabaseMetadataPath(const String & base_path) const;
    ASTPtr getQueryFromMetadata(const String & metadata_path, bool throw_on_error = true) const;
    ASTPtr getCreateQueryFromMetadata(const String & metadata_path, bool throw_on_error) const;


    const String metadata_path;
    /*const*/ String data_path;
};

}