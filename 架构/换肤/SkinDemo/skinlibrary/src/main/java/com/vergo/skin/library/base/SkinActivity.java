package com.vergo.skin.library.base;

import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresApi;
import android.support.v4.view.LayoutInflaterCompat;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.app.AppCompatDelegate;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.vergo.skin.library.SkinManager;
import com.vergo.skin.library.core.ChangeSkinViewInflater;
import com.vergo.skin.library.core.ViewsMatch;
import com.vergo.skin.library.utils.ActionBarUtils;
import com.vergo.skin.library.utils.NavigationUtils;
import com.vergo.skin.library.utils.StatusBarUtils;

/**
 * 换肤Activity父类
 *
 * 用法：
 * 1、继承此类
 * 2、重写openChangeSkin()方法
 */
public class SkinActivity extends AppCompatActivity {

    private ChangeSkinViewInflater viewInflater;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        if(openChangeSkin()) {
            LayoutInflater layoutInflater = LayoutInflater.from(this);
            LayoutInflaterCompat.setFactory2(layoutInflater, this);
        }
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(View parent, String name, Context context, AttributeSet attrs) {
        if (openChangeSkin()) {
            if (viewInflater == null) {
                viewInflater = new ChangeSkinViewInflater();
            }
            return viewInflater.createView(name, context, attrs);
        }
        return super.onCreateView(parent, name, context, attrs);
    }

    /**
     * @return 是否开启换肤，增加此开关是为了避免开发者误继承此父类，导致未知bug
     */
    protected boolean openChangeSkin() {
        return false;
    }

    protected void changeByNightMode(@AppCompatDelegate.NightMode int nightMode) {

        final boolean isPost21 = Build.VERSION.SDK_INT >= 21;

        getDelegate().setLocalNightMode(nightMode);

        if (isPost21 && openChangeSkin()) {
            // 换状态栏
            StatusBarUtils.forStatusBar(this);
            // 换标题栏
            ActionBarUtils.forActionBar(this);
            // 换底部导航栏
            NavigationUtils.forNavigation(this);
        }

        View decorView = getWindow().getDecorView();
        applyViews(decorView);
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    protected void defaultSkin(int themeColorId) {
        this.skinDynamic(null, themeColorId);
    }

    /**
     * 动态换肤（api限制：5.0版本）
     */
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    protected void skinDynamic(String skinPath, int themeColorId) {
        SkinManager.getInstance().loaderSkinResources(skinPath);

        if (themeColorId != 0) {
            int themeColor = SkinManager.getInstance().getColor(themeColorId);
            StatusBarUtils.forStatusBar(this, themeColor);
            NavigationUtils.forNavigation(this, themeColor);
            ActionBarUtils.forActionBar(this, themeColor);
        }

        applyViews(getWindow().getDecorView());
    }

    /**
     * 回调接口 给具体控件换肤操作
     */
    protected void applyViews(View view) {
        if (view instanceof ViewsMatch) {
            ViewsMatch viewsMatch = (ViewsMatch) view;
            viewsMatch.skinnableView();
        }

        if (view instanceof ViewGroup) {
            ViewGroup parent = (ViewGroup) view;
            int childCount = parent.getChildCount();
            for (int i = 0; i < childCount; i++) {
                applyViews(parent.getChildAt(i));
            }
        }
    }
}
