package com.demo.test.ui.base

import android.util.Log

open class BaseClickProxy {

    open fun clickBack() {
        Log.e(TAG, "clickBack: 未实现返回事件")
    }

    open fun clickFinish() {
        Log.e(TAG, "clickFinish: 未实现完成事件")
    }

    companion object {
        private const val TAG = "BaseClickProxy"
    }
}
