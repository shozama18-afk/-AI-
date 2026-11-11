# Copyright (c) 2024-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
"""
順列不変性(permutation invariance)と順列同変性(permutation equivariance)をもったSet Transformer

[Lee 2019] Lee, J., et al. "Set Transformer: A Framework for Attention-based Permutation-Invariant Neural Networks."

著者による実装例（https://github.com/juho-lee/set_transformer）も提供されているが、
maskに対応していないため、torch.nnのMultiHeadAttentionを用いて実装し直す。
SAB,ISAB,PMAのforward()の引数key_padding_maskとして無効トークンのmaskを与えられるようにしている。

第4回空戦AIチャレンジで配布した際にはMultiHeadAttentionの後ろに続くrFF層の実装を失念していたが、
第5回向けサンプルとして新たにrFF層とLayerNormを追加できるようにした。
ln=False,skip_rff=Trueにすることで第4回当時と同じ挙動にでき、重みファイルも使い回せる。

なお、これらの性質は、torch.nnのTransformerEncoderLayerとTransformerDecoderLayerを用いても実現できる。

"""
import torch
import torch.nn as nn

class MAB(nn.MultiheadAttention):
    def __init__(self, embed_dim, num_heads, ln=False, skip_rff=False, **kwargs):
        """
            kwargのうち特にbatch_first,add_bias_kv,add_zero_attnは用途に応じて適切に選択すること。
            特に、長さゼロ(全て無効トークン)のシーケンスが入力される可能性があるならば、add_zero_attnをTrueにする必要がある。
        """
        self.batch_first = kwargs.get('batch_first', True)
        kwargs['batch_first']=self.batch_first
        super().__init__(
            embed_dim,
            num_heads,
            **kwargs
        )

        self.skip_rff = skip_rff
        self.ln = ln
        if self.ln:
            self.ln0 = nn.LayerNorm(embed_dim)
        if not self.skip_rff:
            self.fc_o = nn.Linear(embed_dim, embed_dim)
            if self.ln:
                self.ln1 = nn.LayerNorm(embed_dim)

    def forward(self, X, Y, key_padding_mask=None):
        """
            key_padding_maskは、サイズ(batch_size, embed_dim)のテンソルで与える。
            bool型の場合はTrueのトークンが無視され、float型の場合はattention weightに値がそのまま加算される(-infを与えると無視)。
        """
        ret = super().forward(X, Y, Y,
            key_padding_mask=key_padding_mask,
            need_weights=False,
            attn_mask=None,
            is_causal=False
        )[0]

        if self.ln:
            ret = self.ln0(ret)
        if not self.skip_rff:
            ret = nn.functional.relu(self.fc_o(ret))
            if self.ln:
                ret = self.ln1(ret)

        return ret

class SAB(nn.Module):
    def __init__(self, embed_dim, num_heads, ln=False, skip_rff=False, **kwargs):
        """
            kwargのうち特にbatch_first,add_bias_kv,add_zero_attnは用途に応じて適切に選択すること。
            特に、長さゼロ(全て無効トークン)のシーケンスが入力される可能性があるならば、add_zero_attnをTrueにする必要がある。
        """
        super(SAB, self).__init__()
        self.batch_first = kwargs.get('batch_first', True)
        kwargs['batch_first']=self.batch_first
        self.mab = MAB(
            embed_dim,
            num_heads,
            ln=ln,
            skip_rff=skip_rff,
            **kwargs
        )

    def forward(self, X, key_padding_mask=None):
        """
            key_padding_maskは、サイズ(batch_size, embed_dim)のテンソルで与える。
            bool型の場合はTrueのトークンが無視され、float型の場合はattention weightに値がそのまま加算される(-infを与えると無視)。
        """
        return self.mab(X, X, key_padding_mask=key_padding_mask)

class ISAB(nn.Module):
    def __init__(self, embed_dim, num_heads, num_inds, ln=False, skip_rff=False, **kwargs):
        """
            kwargのうち特にbatch_first,add_bias_kv,add_zero_attnは用途に応じて適切に選択すること。
            特に、長さゼロ(全て無効トークン)のシーケンスが入力される可能性があるならば、add_zero_attnをTrueにする必要がある。
        """
        super(ISAB, self).__init__()
        self.batch_first = kwargs.get('batch_first', True)
        kwargs['batch_first']=self.batch_first
        if self.batch_first:
            self.I = nn.Parameter(torch.Tensor(1, num_inds, embed_dim))
        else:
            self.I = nn.Parameter(torch.Tensor(num_inds, 1, embed_dim))
        nn.init.xavier_uniform_(self.I)
        self.mab0 = MAB(
            embed_dim,
            num_heads,
            ln=ln,
            skip_rff=skip_rff,
            **kwargs
        )
        self.mab1 = MAB(
            embed_dim,
            num_heads,
            ln=ln,
            skip_rff=skip_rff,
            **kwargs
        )

    def forward(self, X, key_padding_mask=None):
        """
            key_padding_maskは、サイズ(batch_size, embed_dim)のテンソルで与える。
            bool型の場合はTrueのトークンが無視され、float型の場合はattention weightに値がそのまま加算される(-infを与えると無視)。
        """
        if self.batch_first:
            I = self.I.repeat(X.size(0), 1, 1)
        else:
            I = self.I.repeat(1, X.size(1), 1)
        H = self.mab0(I, X, key_padding_mask=key_padding_mask)
        return self.mab1(X, H, key_padding_mask=None)

class PMA(nn.Module):
    def __init__(self, embed_dim, num_heads, num_seeds, ln=False, skip_rff=False, **kwargs):
        """
            kwargのうち特にbatch_first,add_bias_kv,add_zero_attnは用途に応じて適切に選択すること。
            特に、長さゼロ(全て無効トークン)のシーケンスが入力される可能性があるならば、add_zero_attnをTrueにする必要がある。
        """
        super(PMA, self).__init__()
        self.num_seeds=num_seeds
        self.batch_first = kwargs.get('batch_first', True)
        kwargs['batch_first']=self.batch_first
        if self.batch_first:
            self.S = nn.Parameter(torch.Tensor(1, num_seeds, embed_dim))
        else:
            self.S = nn.Parameter(torch.Tensor(num_seeds, 1, embed_dim))
        nn.init.xavier_uniform_(self.S)
        self.mab = MAB(
            embed_dim,
            num_heads,
            ln=ln,
            skip_rff=skip_rff,
            **kwargs
        )

    def forward(self, X, key_padding_mask=None):
        """
            key_padding_maskは、サイズ(batch_size, embed_dim)のテンソルで与える。
            bool型の場合はTrueのトークンが無視され、float型の場合はattention weightに値がそのまま加算される(-infを与えると無視)。
        """
        if self.batch_first:
            S = self.S.repeat(X.size(0), 1, 1)
        else:
            S = self.S.repeat(1, X.size(1), 1)
        return self.mab(S, X, key_padding_mask=key_padding_mask)

class PMAWithQuery(nn.Module):
    """
        クエリを外から与えられるようにしたPMA亜種
    """
    def __init__(self, embed_dim, num_heads, ln=False, skip_rff=False, **kwargs):
        """
            kwargのうち特にbatch_first,add_bias_kv,add_zero_attnは用途に応じて適切に選択すること。
            特に、長さゼロ(全て無効トークン)のシーケンスが入力される可能性があるならば、add_zero_attnをTrueにする必要がある。
        """
        super(PMAWithQuery, self).__init__()
        self.mab = MAB(
            embed_dim,
            num_heads,
            ln=ln,
            skip_rff=skip_rff,
            **kwargs
        )

    def forward(self, Q, X, key_padding_mask=None):
        """
            key_padding_maskは、サイズ(batch_size, embed_dim)のテンソルで与える。
            bool型の場合はTrueのトークンが無視され、float型の場合はattention weightに値がそのまま加算される(-infを与えると無視)。
        """
        return self.mab(Q, X, key_padding_mask=key_padding_mask)
