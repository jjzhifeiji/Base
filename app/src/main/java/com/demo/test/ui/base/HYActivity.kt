package com.demo.test.ui.base

import android.os.Bundle
import com.zhifeiji.base.activity.BaseActivity


/**
 * hy activity base
 */
abstract class HYActivity : BaseActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
//        window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
    }


    override fun onDestroy() {
        super.onDestroy()
    }


}