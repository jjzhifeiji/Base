package com.demo.test.ui.base

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.navigation.NavController
import androidx.navigation.fragment.NavHostFragment
import com.zhifeiji.base.activity.BaseFragment

abstract class HYFragment : BaseFragment() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return super.onCreateView(inflater, container, savedInstanceState)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
    }

    /**
     * 为了给所有的fragment，导航跳转fragment的
     * @return
     */
    protected open fun nav(): NavController {
        return NavHostFragment.findNavController(this)
    }

    override fun onStart() {
        super.onStart()
    }

    override fun onResume() {
        super.onResume()
    }

    fun getAdapterView(layout: Int): View {
        val errorView: View = View.inflate(requireContext(), layout, null)
        return errorView
    }
    companion object {
        private const val TAG = "HYFragment"
    }

}