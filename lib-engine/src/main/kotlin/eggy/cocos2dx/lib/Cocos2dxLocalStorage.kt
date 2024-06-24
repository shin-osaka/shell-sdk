/****************************************************************************
 * Copyright (c) 2013-2016 Chukong Technologies Inc.
 * Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 *
 * http://www.cocos2d-x.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package eggy.cocos2dx.lib

import android.content.Context
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper
import eggy.util.LogUtil

object Cocos2dxLocalStorage {
    private const val TAG = "Cocos2dxLocalStorage"
    private var DATABASE_NAME = "jsb.sqlite"
    private var TABLE_NAME = "data"
    private const val DATABASE_VERSION = 1
    private var mDatabaseOpenHelper: DBOpenHelper? = null
    private var mDatabase: SQLiteDatabase? = null

    @JvmStatic
    fun init() {
        init(DATABASE_NAME, TABLE_NAME)
    }

    /**
     * Constructor
     * @param context The Context within which to work, used to create the DB
     * @return
     */
    @JvmStatic
    fun init(dbName: String, tableName: String): Boolean {
        if (Cocos2dxActivity.context != null) {
            DATABASE_NAME = dbName
            TABLE_NAME = tableName
            mDatabaseOpenHelper = DBOpenHelper(Cocos2dxActivity.context)
            mDatabase = mDatabaseOpenHelper!!.writableDatabase
            return true
        }
        return false
    }

    @JvmStatic
    fun destroy() {
        if (mDatabase != null) {
            mDatabase!!.close()
        }
    }

    @JvmStatic
    fun setItem(key: String, value: String) {
        try {
            val sql = "replace into $TABLE_NAME(key,value)values(?,?)"
            mDatabase!!.execSQL(sql, arrayOf<Any>(key, value))
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    fun getItem(key: String): String? {
        var ret: String? = null
        try {
            val sql = "select value from $TABLE_NAME where key=?"
            val c = mDatabase!!.rawQuery(sql, arrayOf(key))
            while (c.moveToNext()) {
                if (ret != null) {
                    break
                }
                val column = c.getColumnIndex("value")
                ret = if (column >= 0) c.getString(column) else null
            }
            c.close()
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return ret
    }

    @JvmStatic
    fun removeItem(key: String) {
        try {
            val sql = "delete from $TABLE_NAME where key=?"
            mDatabase!!.execSQL(sql, arrayOf<Any>(key))
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    fun clear() {
        try {
            val sql = "delete from $TABLE_NAME"
            mDatabase!!.execSQL(sql)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    fun getKey(nIndex: Int): String? {
        var ret: String? = null
        try {
            var nCount = 0
            val sql = "select key from $TABLE_NAME order by rowid asc"
            val c = mDatabase!!.rawQuery(sql, null)
            if (nIndex < 0 || nIndex >= c.count) {
                return null
            }
            while (c.moveToNext()) {
                if (nCount == nIndex) {
                    val column = c.getColumnIndex("key")
                    ret = if (column >= 0) c.getString(column) else null
                    break
                }
                nCount++
            }
            c.close()
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return ret
    }

    @JvmStatic
    val length: Int
        get() {
            var res = 0
            try {
                val sql = "select count(*) as nums from $TABLE_NAME"
                val c = mDatabase!!.rawQuery(sql, null)
                if (c.moveToNext()) {
                    val column = c.getColumnIndex("nums")
                    res = if (column >= 0) c.getInt(column) else 0
                }
                c.close()
            } catch (e: Exception) {
                e.printStackTrace()
            }
            return res
        }

    /**
     * This creates/opens the database.
     */
    private class DBOpenHelper(context: Context?) :
        SQLiteOpenHelper(context, DATABASE_NAME, null, DATABASE_VERSION) {
        override fun onCreate(db: SQLiteDatabase) {
            db.execSQL("CREATE TABLE IF NOT EXISTS $TABLE_NAME(key TEXT PRIMARY KEY,value TEXT);")
        }

        override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) {
            LogUtil.w(
                TAG, "Upgrading database from version " + oldVersion + " to "
                        + newVersion + ", which will destroy all old data"
            )
        }
    }
}