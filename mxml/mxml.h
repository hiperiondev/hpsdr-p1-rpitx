
/** Lightweight, in-memory XML parser */
struct mxml;

/**
 * Creates an XML parser/editor context
 * Assumes that the XML source:
 * <ul><li>Is well-formed, and tags balance
 *     <li>Does not use "<tag/>"-style empty tags
 *     <li>Does not use attributes
 *     <li>Is encoded in UTF-8 or ASCII
 *     <li>Only uses entities &lt; &amp; &gt;
 *     <li>Only has text in leaf elements
 * </ul>
 * @param base Pointer to read-only, in-memory XML source.
 * @param len Length of the XML source in bytes.
 * @returns a context structure. Free it with #mxml_close().
 * @retval NULL on error, sets #errno.
 */
struct mxml *mxml_new(const char *base, unsigned int len);

/**
 * Closes the XML file opened by #xml_open().
 * All edits made will be lost
 */
void mxml_free(struct mxml *m);

/**
 * Gets the text value of an XML element.
 * Prior edits are honoured.
 * @param key The search key for the leaf element.
 *            The key is a '.'-delimited sequence of child tag names.  The
 *            first matching tag is found.
 *	      The subkey "tag[5]" is converted to "tags.tag5".
 *	      The special key "tag[#]" is converted to "tags.total".
 *	      The special key "tag[$]" expands to "tags.tagN" where "N"
 *	      is the value of "tags.total".
 *	      The following chars are not permitted in tag names: . # % [
 * @returns the unescaped content in a string allocated by malloc
 * @retval NULL [ENOENT] the key does not exist
 * @retval NULL [EINVAL] the key was malformed
 * @retval NULL [ENOMEM] the key was too long
 * @retval NULL [ENOMEM] not enough memory to copy the value
 */
char *mxml_get(struct mxml *m, const char *key);

/**
 * Tests if the tag described by the key exists.
 * @retval 0  the key does not exist
 * @retval 0  the key is not well-formed
 * @retval 1  the key does exist
 */
int mxml_exists(struct mxml *m, const char *key);

/**
 * Deletes the element (and its children) from the document.
 * @param key the key to delete.
 *            If the key ends with "[$]" then the total is updated.
 *            If the key ends with "[*]" then a whole list is deleted.
 * @retval 0 successfully deleted
 * @retval 0 element did not exist
 * @retval -1 [ENOMEM] out of memory
 * @retval -1 [EINVAL] the key is malformed
 */
int mxml_delete(struct mxml *m, const char *key);

/**
 * Updates the text value of an existing element.
 * @retval 0  success
 * @retval -1 [ENOENT] the element has been deleted or never existed.
 * @retval -1 [EINVAL] the key is malformed
 * @retval -1 [ENOMEM] out of memory
 */
int mxml_update(struct mxml *m, const char *key, const char *value);

/**
 * Appends a new tag to its parent, creating parents as needed.
 * @param key the tag to append.
 *            If the key contains a single "[+]" then the corresponding
 *            tag.total is incremented and its new number is used.
 *            Subsequent calls should then refer to it as "[$]".
 * @param value (optional) the value of the tag; or NULL
 * @retval 0  success
 * @retval -1 [EXIST] the key already exists, and was not changed.
 * @retval -1 [EINVAL] the key is malformed
 * @retval -1 [ENOMEM] out of memory
 */
int mxml_append(struct mxml *m, const char *key, const char *value);

/**
 * Updates, creates or deletes an element.
 * If the value is NULL, then this function is the same as #mxml_delete().
 * If the key already exists, this function is the same as #mxml_update().
 * Otherwise this function is the same as #mxml_append().
 * @param key   The key to set.
 * @param value The text value to use; NULL means to delete.
 * @retval 0  success
 * @retval -1 [EINVAL] the key is malformed
 * @retval -1 [ENOMEM] out of memory
 */
int mxml_set(struct mxml *m, const char *key, const char *value);

/**
 * Expands a key containing [$] into its [integer] form.
 * @param key  the tag to expand
 * @return a malloc-provided string, containing the key but
 *         with [$] replaced by [<integer>],
 * @retval NULL [ENOMEM] cannot allocate memory
 * @retval NULL [EINVAL] the key is malformed
 * @retval NULL [ENOENT] the key is malformed
 */
char *mxml_expand_key(struct mxml *m, const char *key);

/**
 * Writes out an XML document with edits.
 * @param writefn Output callback function. If the callback function
 *                returns -1, then this function aborts and returns -1 too.
 * @param context Context value passed to @a writefn.
 * @returns the sum of the return values from @a writefn.
 * @retval -1 if @a writefn returned -1
 */
int mxml_write(const struct mxml *m,
	int (*writefn)(void *context, const char *text, unsigned int len),
	void *context);

/**
 * Extract a list of all the keys in the document.
 * The key list is derived from the XML document and edits, and
 * is returned in the same order as #mxml_write() would emit them.
 * Empty and interior (container) keys are also returned in the list.
 * @param nkeys_return storage for returning the number of keys returned.
 *                     This storage is always set.
 * @returns An array of expanded key strings.
 *          The array and each string should be released with #free().
 * @retval NULL [ENOMEM] could not allocate memory
 */
char **mxml_keys(const struct mxml *m, unsigned int *nkeys_return);

/**
 * Frees keys returned by #mxml_keys().
 * @param keys (optional) array of keys. NULL is acceptable.
 */
void mxml_free_keys(char **keys, unsigned int nkeys);
