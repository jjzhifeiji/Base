package com.zhifeiji.base.application

import android.app.Application
import android.content.Context
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.ViewModelStore
import androidx.lifecycle.ViewModelStoreOwner
import com.blankj.utilcode.util.Utils

open class BaseApplication : Application(), ViewModelStoreOwner {


    companion object {
        lateinit var appContext: Context
        lateinit var mAppViewModelStore: ViewModelStore
        var sDebug = false
    }

    override val viewModelStore: ViewModelStore
        get() = mAppViewModelStore

    fun setDebug(debug: Boolean) {
        sDebug = debug
    }

    override fun onCreate() {
        super.onCreate()
        appContext = this
        Utils.init(this);
        mAppViewModelStore = ViewModelStore()

    }

    open fun getAppViewModelProvider(): ViewModelProvider {
        return ViewModelProvider(this, getAppFactory())
    }

    open fun getAppFactory(): ViewModelProvider.Factory {
        return ViewModelProvider.AndroidViewModelFactory.getInstance(this)
    }


}
