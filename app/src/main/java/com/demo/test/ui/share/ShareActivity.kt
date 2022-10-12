package com.demo.test.ui.share

import android.Manifest
import android.os.Bundle
import com.blankj.utilcode.util.LogUtils
import com.blankj.utilcode.util.ToastUtils
import com.hjq.permissions.OnPermissionCallback
import com.hjq.permissions.XXPermissions
import com.demo.test.ui.base.HYActivity
import com.hy.dihome.databinding.ActivityShareBinding

class ShareActivity : HYActivity() {


    private lateinit var dataBinding: ActivityShareBinding


    //运行时候需要的所有权限
    private val permissions = arrayOf(
        Manifest.permission.CAMERA,
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.ACCESS_FINE_LOCATION,
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.ACCESS_COARSE_LOCATION,
        Manifest.permission.READ_PHONE_STATE,
        Manifest.permission.REQUEST_INSTALL_PACKAGES
    )


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        dataBinding = ActivityShareBinding.inflate(layoutInflater)
        dataBinding.lifecycleOwner = this
        setContentView(dataBinding.root)
//        // 问题及方案：https://www.cnblogs.com/net168/p/5722752.html
//        // 如果当前 Activity 不是任务栈中的第一个 Activity
//        if (!isTaskRoot) {
//            val intent = intent
//            // 如果当前 Activity 是通过桌面图标启动进入的
//            if (intent != null && intent.hasCategory(Intent.CATEGORY_LAUNCHER)
//                && Intent.ACTION_MAIN == intent.action
//            ) {
//                // 对当前 Activity 执行销毁操作，避免重复实例化入口
//                finish()
//                return
//            }
//        }

        initView()
    }

    private fun initView() {

    }

    private fun go() {

    }

    private fun checkPermission() {
        LogUtils.d("onCreate PermissionsAspect aroundJoinPoint")

        XXPermissions.with(this)
            .permission(permissions)
            .request(object : OnPermissionCallback {
                override fun onGranted(permissions: List<String>, all: Boolean) {
                    if (!all) {
                        ToastUtils.showShort("获取部分权限成功，但部分权限未正常授予")
                    } else {
                        go()
                    }
                }

                override fun onDenied(permissions: List<String>, never: Boolean) {
                    if (never) {
                        ToastUtils.showShort("被永久拒绝授权")
                        // 如果是被永久拒绝就跳转到应用权限系统设置页面
                        XXPermissions.startPermissionActivity(this@ShareActivity, permissions)
                    } else {
                        ToastUtils.showShort("获取权限失败")
                    }
                }
            })
    }

    companion object {
        private const val TAG = "ShareActivity"
    }

}