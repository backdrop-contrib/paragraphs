# Layout Paragraphs

Heavily under development.

@todo:

 - Can all layouts start out with a pre-named default content region that
   can't be removed? This can be the fallback/refuge region, and could be
   displayed slightly differently than the rest.
 - Save/load selected layout template and block data
 - Paragraph blocks with permissions, access for various types of Paragraphs
 - Front end display; incl check modal edits.
 - Populate regions automatically; incl. when switching the layout template,
   and when loading the page first -- with existing LP blocks and also checking
   if there are any Paragraph items in the database that aren't in blocks yet
   (flow them into the default region).
 - Clear entries in LP when field instance is deleted.
 - Access/permissions.
 - Reorder & save blocks; double check
 - Limit # blocks by cardinality of field
 - Revisions work
 - Nested paragraphs? Would that yield nested layouts, or restrict layouts to
   the top level only?
 - Validation errors on save cause display issues
 - Multiple Paragraphs fields on an entity
 - Entity type agnostic testing
 - Test inserting new entities
 - Ajax layout template change on user/term/other?
