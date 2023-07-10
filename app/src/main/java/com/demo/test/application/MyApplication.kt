package com.demo.test.application

import android.content.Context
import androidx.core.content.ContextCompat
import com.blankj.utilcode.util.ToastUtils
import com.blankj.utilcode.util.Utils
import com.demo.test.viewmodel.AppViewModel
import com.hy.dihome.R
import com.zhifeiji.base.application.BaseApplication
import com.zhifeiji.base.manager.ActivityManager


open class MyApplication : BaseApplication() {


    companion object {
        lateinit var appViewModel: AppViewModel
        lateinit var appContext: Context
    }

    init {
    }

    override fun onCreate() {
        super.onCreate()
        appContext = this
        initUtils()
        initAppViewModel()
        initTitleBar()
        initDataBase()
        initConstant()
        initHttp()
        initDebug()
        initUM()
        // Activity 栈管理初始化
        ActivityManager.getInstance().init(this)
        initFinish()
    }


    private fun initUtils() {
        Utils.init(this)
    }

    private fun initUM() {

    }


    private fun initFinish() {

    }

    private fun initAppViewModel() {
        appViewModel = getAppViewModelProvider()[AppViewModel::class.java]
    }

    private fun initDebug() {
    }

    private fun initHttp() {

    }

    private fun initTitleBar() {
        // 初始化 TitleBar 样式
    }

    private fun initConstant() {
    }

    private fun initDataBase() {
    }

}